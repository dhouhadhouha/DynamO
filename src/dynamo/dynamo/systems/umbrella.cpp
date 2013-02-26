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

#include <dynamo/systems/umbrella.hpp>

#include <dynamo/units/units.hpp>
#include <dynamo/BC/BC.hpp>
#include <dynamo/particle.hpp>
#include <dynamo/species/species.hpp>
#include <dynamo/NparticleEventData.hpp>
#include <dynamo/ranges/include.hpp>
#include <dynamo/dynamics/dynamics.hpp>
#include <dynamo/schedulers/scheduler.hpp>
#include <dynamo/outputplugins/outputplugin.hpp>
#include <magnet/xmlwriter.hpp>
#include <magnet/xmlreader.hpp>
#include <boost/lexical_cast.hpp>

#ifdef DYNAMO_DEBUG 
#include <boost/math/special_functions/fpclassify.hpp>
#endif

namespace dynamo {

  SysUmbrella::SysUmbrella(const magnet::xml::Node& XML, dynamo::Simulation* tmp): 
    System(tmp),
    a(1.0),
    b(1.0),
    delU(0.1),
    ulevelcenter(0),
    ulevel(-1),
    ulevelset(false)
  {
    dt = HUGE_VAL;
    operator<<(XML);
    type = UMBRELLA;
  }

  SysUmbrella::SysUmbrella(dynamo::Simulation* nSim, double na, double nb, double ndelu, 
			 std::string nName, IDRange* r1, IDRange* r2):
    System(nSim),
    a(na),
    b(nb),
    delU(ndelu),
    ulevelcenter(0),
    ulevel(-1),
    ulevelset(false),
    range1(r1),
    range2(r2)
  {
    sysName = nName;
    type = UMBRELLA;
  }

  void 
  SysUmbrella::runEvent() const
  {
    double locdt = dt;
  
#ifdef DYNAMO_DEBUG 
    if (boost::math::isnan(locdt))
      M_throw() << "A NAN system event time has been found";
#endif
  
    Sim->systemTime += locdt;
  
    Sim->ptrScheduler->stream(locdt);
  
    //dynamics must be updated first
    Sim->stream(locdt);

    ++Sim->eventCount;

    for (const size_t& id : *range1)
      Sim->dynamics->updateParticle(Sim->particles[id]);
  
    for (const size_t& id : *range2)
      Sim->dynamics->updateParticle(Sim->particles[id]);
  
    bool kedown(false); //Will kinetic energy go down?

    int newulevel ;

    if (ulevel == 0)
      {
	kedown = true;
      
	if (type == STEP_OUT)
	  newulevel = 1;
	else
	  newulevel = -1;
      }
    else if (type == STEP_OUT)
      {
	if (ulevel > 0) kedown = true;
	newulevel = ulevel + 1; 
      }
    else //if (type == STEP_IN)
      {
	if (ulevel < 0) kedown = true;
	newulevel = ulevel - 1;
      }
    
    EEventType etype(NONE);

    NEventData SDat(Sim->dynamics->multibdyWellEvent
		    (*range1, *range2, 0.0, (kedown) ? -delU : delU, etype));

    if (etype != BOUNCE)
      ulevel = newulevel;

    (*Sim->_sigParticleUpdate)(SDat);
  
    //Only 1ParticleEvents occur
    for (const ParticleEventData& PDat : SDat.L1partChanges)
      Sim->ptrScheduler->fullUpdate(Sim->particles[PDat.getParticleID()]);
  
    for (shared_ptr<OutputPlugin>& Ptr : Sim->outputPlugins)
      Ptr->eventUpdate(*this, SDat, locdt); 
  }

  void
  SysUmbrella::initialise(size_t nID)
  {
    ID = nID;

    for (const size_t& id : *range1)
      Sim->dynamics->updateParticle(Sim->particles[id]);
  
    for (const size_t& id : *range2)
      Sim->dynamics->updateParticle(Sim->particles[id]);
  
    ulevelcenter = int( - a * b * b / delU);
    
    std::pair<Vector, Vector> r1data = Sim->dynamics->getCOMPosVel(*range1);
    std::pair<Vector, Vector> r2data = Sim->dynamics->getCOMPosVel(*range2);
    Vector r12 = r1data.first - r2data.first;
    Sim->BCs->applyBC(r12);

    double r = r12.nrm();
    
    if (!ulevelset)
      {
	ulevel = int(a * (r - b) * (r - b) / delU);
	if (r < b) ulevel *= -1;
	ulevelset = true;
      }
  
    recalculateTime();

    (*Sim->_sigParticleUpdate).connect<SysUmbrella, &SysUmbrella::particlesUpdated>(this);
  }

  void 
  SysUmbrella::recalculateTime()
  {
    for (const size_t& id : *range1)
      Sim->dynamics->updateParticle(Sim->particles[id]);
  
    for (const size_t& id : *range2)
      Sim->dynamics->updateParticle(Sim->particles[id]);
  
    double R_max, R_min;

    dt = HUGE_VAL;
    type = NONE;

    if (ulevel == ulevelcenter)
      {
	R_max = b - sqrt((ulevel * delU) / a);      
    
	if (b==0)//Allow a double width well if b==0
	  R_max = b + sqrt((ulevel + 1 * delU) / a);
      
	dt = Sim->dynamics->SphereSphereOutRoot(*range1, *range2, R_max);
	
	if (dt != HUGE_VAL)
	  type = STEP_OUT;
      
	return;
      }
  
    if (ulevel == 0)
      {
	//We're on the minimum

	//We don't worry about the minimum crossing r=0, as this is
	//caught by the above if statement
      
	R_max = b + sqrt((1 * delU) / a);
	R_min = b - sqrt((1 * delU) / a);
      }
    else if (ulevel < 0)
      {
	R_max = b - sqrt((-ulevel) * delU / a);
	R_min = b - sqrt(((-ulevel) + 1) * delU / a);
      }
    else
      {
	R_min = b + sqrt((ulevel * delU) / a);
	R_max = b + sqrt(((ulevel + 1) * delU) / a);
      }

    dt = Sim->dynamics->SphereSphereInRoot(*range1, *range2, R_min);
    
    if (dt != HUGE_VAL)
      {
	type = STEP_IN;
	return;
      }

    dt = Sim->dynamics->SphereSphereOutRoot(*range1, *range2, R_max);

    if (dt != HUGE_VAL)
      {
	type = STEP_OUT;
	return;
      }
  }

  void 
  SysUmbrella::particlesUpdated(const NEventData& PDat)
  {
    for (const ParticleEventData& pdat : PDat.L1partChanges)
      if (range1->isInRange(Sim->particles[pdat.getParticleID()])
	  || range2->isInRange(Sim->particles[pdat.getParticleID()]))
	{
	  recalculateTime();
	  Sim->ptrScheduler->rebuildSystemEvents();
	  return;
	}

    for (const PairEventData& pdat : PDat.L2partChanges)
      if (range1->isInRange(Sim->particles[pdat.particle1_.getParticleID()])
	  || range2->isInRange(Sim->particles[pdat.particle1_.getParticleID()])
	  || range1->isInRange(Sim->particles[pdat.particle2_.getParticleID()])
	  || range2->isInRange(Sim->particles[pdat.particle2_.getParticleID()]))
	{
	  recalculateTime();
	  Sim->ptrScheduler->rebuildSystemEvents();
	  return;
	}
  }

  void
  SysUmbrella::operator<<(const magnet::xml::Node& XML)
  {
    try {
      sysName = XML.getAttribute("Name");

      a = XML.getAttribute("a").as<double>()
	* Sim->units.unitEnergy() 
	/ Sim->units.unitArea();

      b = XML.getAttribute("b").as<double>()
	* Sim->units.unitLength();

      delU = XML.getAttribute("delU").as<double>() * Sim->units.unitEnergy();
      range1 = shared_ptr<IDRange>(IDRange::getClass(XML.getNode("Range1"), Sim));
      range2 = shared_ptr<IDRange>(IDRange::getClass(XML.getNode("Range2"), Sim));
    
      if (XML.hasAttribute("currentulevel"))
	{
	  ulevel = XML.getAttribute("currentulevel").as<size_t>();
	  ulevelset = true;
	}
    
    }
    catch (boost::bad_lexical_cast &)
      { M_throw() << "Failed a lexical cast in SysUmbrella"; }
  }

  void 
  SysUmbrella::outputXML(magnet::xml::XmlStream& XML) const
  {
    XML << magnet::xml::tag("System")
	<< magnet::xml::attr("Type") << "Umbrella"
	<< magnet::xml::attr("a") << a * Sim->units.unitArea() 
      / Sim->units.unitEnergy()
	<< magnet::xml::attr("b") << b / Sim->units.unitLength()
	<< magnet::xml::attr("delU") << delU / Sim->units.unitEnergy()
	<< magnet::xml::attr("currentulevel") << ulevel
	<< magnet::xml::attr("Name") << sysName
	<< magnet::xml::tag("Range1")
	<< range1
	<< magnet::xml::endtag("Range1")
	<< magnet::xml::tag("Range2")
	<< range2
	<< magnet::xml::endtag("Range2")
	<< magnet::xml::endtag("System");
  }
}
