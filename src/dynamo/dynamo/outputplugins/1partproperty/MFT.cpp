/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.dynamomd.org
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <dynamo/outputplugins/1partproperty/MFT.hpp>
#include <dynamo/simulation.hpp>

#include <dynamo/species/species.hpp>
#include <dynamo/1particleEventData.hpp>
#include <dynamo/units/units.hpp>
#include <magnet/xmlwriter.hpp>
#include <magnet/xmlreader.hpp>

namespace dynamo {
  OPMFT::OPMFT(const dynamo::Simulation* tmp, const magnet::xml::Node& XML):
    OP1PP(tmp,"MeanFreeLength", 250),
    collisionHistoryLength(10),
    binwidth(0.01)
  {
    operator<<(XML);
  }

  void 
  OPMFT::operator<<(const magnet::xml::Node& XML)
  {
    try 
      {
	if (XML.hasAttribute("BinWidth"))
	  binwidth = XML.getAttribute("BinWidth").as<double>();

	if (XML.hasAttribute("Length"))
	  collisionHistoryLength = XML.getAttribute("Length").as<size_t>();
      }
    catch (boost::bad_lexical_cast&)
      {
	M_throw() << "Failed a lexical cast in OPMFL";
      }
  }

  void
  OPMFT::initialise()
  {
    lastTime.resize(Sim->N, 
		    boost::circular_buffer<double>(collisionHistoryLength, 0.0));
  
    std::vector<magnet::math::Histogram<> > vecTemp;
  
    vecTemp.resize(collisionHistoryLength, 
		   magnet::math::Histogram<>(Sim->units.unitTime() * binwidth));
  
    data.resize(Sim->species.size(), vecTemp);
  }

  void 
  OPMFT::A1ParticleChange(const ParticleEventData& PDat)
  {
    //We ignore stuff that hasn't had an event yet

    for (size_t collN = 0; collN < collisionHistoryLength; ++collN)
      if (lastTime[PDat.getParticleID()][collN] != 0.0)
	{
	  data[PDat.getSpeciesID()][collN]
	    .addVal(Sim->systemTime 
		    - lastTime[PDat.getParticleID()][collN]);
	}
  
    lastTime[PDat.getParticleID()].push_front(Sim->systemTime);
  }

  void
  OPMFT::output(magnet::xml::XmlStream &XML)
  {
    XML << magnet::xml::tag("MFT");
  
    for (size_t id = 0; id < data.size(); ++id)
      {
	XML << magnet::xml::tag("Species")
	    << magnet::xml::attr("Name")
	    << Sim->species[id]->getName();
      
	for (size_t collN = 0; collN < collisionHistoryLength; ++collN)
	  {
	    XML << magnet::xml::tag("Collisions")
		<< magnet::xml::attr("val") << collN + 1;
	  
	    data[id][collN].outputHistogram
	      (XML, 1.0 / Sim->units.unitTime());
	  
	    XML << magnet::xml::endtag("Collisions");
	  }
	
	XML << magnet::xml::endtag("Species");
      }
  
    XML << magnet::xml::endtag("MFT");
  }
}
