/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2008  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include "fastsingle.hpp"
#include "../extcode/threadpool.hpp"
#include <boost/foreach.hpp>
#include "../dynamics/interactions/intEvent.hpp"
#include "../simulation/particle.hpp"
#include "../dynamics/dynamics.hpp"
#include "../base/is_simdata.hpp"
#include "streamtask.hpp"
#include "../dynamics/globals/globEvent.hpp"
#include "../dynamics/systems/system.hpp"
#include "../extcode/xmlParser.h"

CSFastSingle::CSFastSingle(const XMLNode& XML, const DYNAMO::SimData* Sim):
  CScheduler(Sim,"SingleCollList")
{
  operator<<(XML);
  I_cout() << "Fast single collision list loaded";
}

CSFastSingle::CSFastSingle(const DYNAMO::SimData* Sim):
  CScheduler(Sim,"SingleCollList")
{
  I_cout() << "Fast single collision list loaded";
}

void 
CSFastSingle::rescaleTimes(Iflt scale) 
{   
  if (!globEventQueue.empty())
    BOOST_FOREACH(CGlobEvent& event, globEventQueue)
      event.scaleTime(scale);

  //Now rescale all the times
  BOOST_FOREACH(CIntEvent& PDat, intEventQueue)
    PDat.scaleTime(scale);
}


void 
CSFastSingle::update(const CParticle& part)
{  

  //Update global event
  if (!globEventQueue.empty())
    globEventQueue[part.getID()] = getGlobEvent(part);


  //Update any invalid events where this is part==particle2
  for (size_t id = 0; id < part.getID(); ++id)
    if (intEventQueue[id].getParticle2() == part)
      {
	rebuildCollision(intEventQueue[id]);
      }
    else
      {
	CIntEvent tmpcoll 
	  = Sim->Dynamics.getEvent(Sim->vParticleList[id], part);
	if (tmpcoll < intEventQueue[id])
	  intEventQueue[id] = tmpcoll;
      }

  //Update any events where this is part==particle1
  rebuildCollision(intEventQueue[part.getID()]);
}

ENextEvent
CSFastSingle::nextEventType() const
{
  Iflt tmpt = HUGE_VAL, tmpt2;
  ENextEvent tmpType = Interaction;

  /*std::cerr << "\nCurrent list state";
  size_t id = 0;
  BOOST_FOREACH(const CIntEvent& eevent, intEventQueue)
    std::cerr << "\n ID = " << id++ << " ID1 = " 
	      << eevent.getParticle1().getID();

	      std::cerr << "\nEND Current list state";*/

  nextIntEvent = std::min_element(intEventQueue.begin(), intEventQueue.end());
  nextGlobEvent = std::min_element(globEventQueue.begin(), globEventQueue.end());

  if (!globEventQueue.empty())
    if ((tmpt2 = nextGlobEvent->getdt()) < tmpt)
      {
	tmpType = Global;
	tmpt = tmpt2;
      }

  if (!Sim->Dynamics.getSystemEvents().empty())
    if ((tmpt2 = (*min_element(Sim->Dynamics.getSystemEvents().begin(), 
			       Sim->Dynamics.getSystemEvents().end()))->getdt())
	< tmpt)
    {
      	tmpType = System;
	tmpt = tmpt2;
    }

  if (nextIntEvent->getdt() < tmpt)
    tmpType = Interaction;

  return tmpType;
}

const CIntEvent 
CSFastSingle::earliestIntEvent() const
{
  return *nextIntEvent;
}

const CGlobEvent 
CSFastSingle::earliestGlobEvent() const
{
  return *nextGlobEvent;
}

void
CSFastSingle::initialise()
{
  if (Sim->lN < 2)
    I_throw() << "Not enough particles (need at least 2)";
  
  if (intEventQueue.size())
    I_throw() << "Collision list Re-initialised!";

  initGlobalQueue();
  rebuildList();
}

void 
CSFastSingle::stream(const Iflt dt)
{
  BOOST_FOREACH(CIntEvent& event,intEventQueue)
    event.incrementTime(dt);
      
  if (!globEventQueue.empty())
    BOOST_FOREACH(CGlobEvent& event, globEventQueue)
      event.incrementTime(dt);
}

void 
CSFastSingle::initGlobalQueue()
{
  //First check if there are any globals
  if (!(Sim->Dynamics.getGlobals().size()))
      return;

  CGlobEvent e1,e2;
  //Ok, so lets test each particle against the global
  globEventQueue.clear();
  globEventQueue.resize(Sim->lN);

  BOOST_FOREACH(const CParticle& part, Sim->vParticleList)
    globEventQueue[part.getID()] = getGlobEvent(part);
}

void
CSFastSingle::rebuildList()
{
  intEventQueue.clear();
  intEventQueue.resize(Sim->lN);
  std::vector<CIntEvent> eList;
  
  BOOST_FOREACH(const CParticle & Part1, Sim->vParticleList)  
    {
      intEventQueue[Part1.getID()] = CIntEvent(Part1);
      rebuildCollision(intEventQueue[Part1.getID()]);
    }
}

void 
CSFastSingle::rebuildCollision(CIntEvent & coll)
{
  coll.invalidate();

  for (std::vector<CParticle>::const_iterator iPtr 
	 = Sim->vParticleList.begin() + (coll.getParticle1().getID() + 1);
       iPtr !=  Sim->vParticleList.end();
       ++iPtr)
    {
      CIntEvent tmpColl = Sim->Dynamics.getEvent(coll.getParticle1(), *iPtr);
      if (coll > tmpColl)
	coll = tmpColl;
    }
}

void 
CSFastSingle::operator<<(const XMLNode&)
{}

void 
CSFastSingle::outputXML(xmlw::XmlStream& XML) const
{
  XML << xmlw::attr("Type") << "FastSingle";
}
