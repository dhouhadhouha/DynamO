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

#include <dynamo/interactions/captures.hpp>
#include <dynamo/particle.hpp>
#include <dynamo/schedulers/scheduler.hpp>
#include <dynamo/simulation.hpp>
#include <magnet/xmlwriter.hpp>
#include <magnet/xmlreader.hpp>

namespace dynamo {
  void 
  ISingleCapture::testAddToCaptureMap(const Particle& p1, const size_t& p2) const
  {
    if (captureTest(p1, Sim->particles[p2]))
      addToCaptureMap(p1, Sim->particles[p2]);
  }   


  void 
  ICapture::initCaptureMap()
  {
    //If not loaded or invalidated
    if (noXmlLoad)
      {      
	clear();

	for (std::vector<Particle>::const_iterator iPtr1 = Sim->particles.begin();
	     iPtr1 != Sim->particles.end(); iPtr1++)
	  for (std::vector<Particle>::const_iterator iPtr2 = iPtr1+1;
	       iPtr2 != Sim->particles.end(); iPtr2++)
	    //Check this interaction is the correct interaction for the pair
	    if (Sim->getInteraction(*iPtr1, *iPtr2).get() == static_cast<const Interaction*>(this))
	      testAddToCaptureMap(*iPtr1, iPtr2->getID());
      }
  }

  void 
  ISingleCapture::loadCaptureMap(const magnet::xml::Node& XML)
  {
    if (XML.hasNode("CaptureMap"))
      {
	noXmlLoad = false;
	clear();

	for (magnet::xml::Node node = XML.getNode("CaptureMap").fastGetNode("Pair");
	     node.valid(); ++node)
	  captureMap.insert(cMapKey(node.getAttribute("ID1").as<size_t>(),
				    node.getAttribute("ID2").as<size_t>()));
      }
  }

  void 
  ISingleCapture::outputCaptureMap(magnet::xml::XmlStream& XML) const 
  {
    XML << magnet::xml::tag("CaptureMap");

    for (const cMapKey& IDs : captureMap)
      XML << magnet::xml::tag("Pair")
	  << magnet::xml::attr("ID1") << IDs.first
	  << magnet::xml::attr("ID2") << IDs.second
	  << magnet::xml::endtag("Pair");
  
    XML << magnet::xml::endtag("CaptureMap");
  }

  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////

  void 
  IMultiCapture::testAddToCaptureMap(const Particle& p1, const size_t& p2) const
  {
    int capval = captureTest(p1, Sim->particles[p2]);
    if (capval) captureMap[cMapKey(p1.getID(), p2)] = capval; 
  }

  void 
  IMultiCapture::loadCaptureMap(const magnet::xml::Node& XML)
  {
    if (XML.hasNode("CaptureMap"))
      {
	noXmlLoad = false;
	clear();

	for (magnet::xml::Node node = XML.getNode("CaptureMap").fastGetNode("Pair");
	     node.valid(); ++node)
	  captureMap[cMapKey(node.getAttribute("ID1").as<size_t>(),
			     node.getAttribute("ID2").as<size_t>())]
	    = node.getAttribute("val").as<size_t>();
      }
  }

  void 
  IMultiCapture::outputCaptureMap(magnet::xml::XmlStream& XML) const 
  {
    XML << magnet::xml::tag("CaptureMap");

    typedef std::pair<const cMapKey, int> locpair;

    for (const locpair& IDs : captureMap)
      XML << magnet::xml::tag("Pair")
	  << magnet::xml::attr("ID1") << IDs.first.first
	  << magnet::xml::attr("ID2") << IDs.first.second
	  << magnet::xml::attr("val") << IDs.second
	  << magnet::xml::endtag("Pair");
  
    XML << magnet::xml::endtag("CaptureMap");
  }

  size_t
  ISingleCapture::validateState(bool textoutput, size_t max_reports) const
  {
    size_t retval(0);
    for (const cMapKey& IDs : captureMap)
      {
	const Particle& p1(Sim->particles[IDs.first]);
	const Particle& p2(Sim->particles[IDs.second]);

	shared_ptr<Interaction> interaction_ptr = Sim->getInteraction(p1, p2);
	if (interaction_ptr.get() == static_cast<const Interaction*>(this))
	  retval += interaction_ptr->validateState(p1, p2, retval < max_reports);
	else
	  derr << "Particle " << p1.getID() << " and Particle " << p2.getID()
	       << " are in the capture map of the \"" << intName << "\" interaction, but this is not the corresponding interaction for that pair!"
	       << " They are handled by the \"" << interaction_ptr->getName() << "\" Interaction"
	       << std::endl;
      }

    return retval;
  }
  
  size_t
  IMultiCapture::validateState(bool textoutput, size_t max_reports) const
  {
    size_t retval(0);
    typedef std::pair<const dynamo::ICapture::cMapKey, int> mapdata;
    for (const mapdata& IDs : captureMap)
      {
	const Particle& p1(Sim->particles[IDs.first.first]);
	const Particle& p2(Sim->particles[IDs.first.second]);

	shared_ptr<Interaction> interaction_ptr = Sim->getInteraction(p1, p2);
	if (interaction_ptr.get() == static_cast<const Interaction*>(this))
	  retval += interaction_ptr->validateState(p1, p2, retval < max_reports);
	else
	  derr << "Particle " << p1.getID() << " and Particle " << p2.getID()
	       << " are in the capture map of the \"" << intName << "\" interaction, but this is not the corresponding interaction for that pair!"
	       << " They are handled by the \"" << interaction_ptr->getName() << "\" Interaction"
	       << std::endl;
      }

    return retval;
  }

}
