/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2010  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include "roughhardsphere.hpp"
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <iomanip>
#include "../../base/is_exception.hpp"
#include "../../extcode/xmlwriter.hpp"
#include "../../extcode/xmlParser.h"
#include "../../dynamics/interactions/intEvent.hpp"
#include "../liouvillean/liouvillean.hpp"
#include "../units/units.hpp"
#include "../../base/is_simdata.hpp"
#include "../2particleEventData.hpp"
#include "../BC/BC.hpp"
#include <sstream>
#include "../ranges/1range.hpp"
#include "../../schedulers/scheduler.hpp"
#include "../NparticleEventData.hpp"
#include "../liouvillean/CompressionL.hpp"

IRoughHardSphere::IRoughHardSphere(DYNAMO::SimData* tmp, Iflt nd, 
			   Iflt ne, C2Range* nR):
  CInteraction(tmp, nR),
  diameter(nd), d2(nd*nd), e(ne) {}

IRoughHardSphere::IRoughHardSphere(const XMLNode& XML, DYNAMO::SimData* tmp):
  CInteraction(tmp,NULL)
{
  operator<<(XML);
}

void 
IRoughHardSphere::initialise(size_t nID)
{ ID=nID; }

void 
IRoughHardSphere::operator<<(const XMLNode& XML)
{ 
  if (strcmp(XML.getAttribute("Type"),"HardSphere"))
    D_throw() << "Attempting to load Hardsphere from non hardsphere entry";
  
  range.set_ptr(C2Range::loadClass(XML,Sim));
  
  try 
    {
      diameter = Sim->dynamics.units().unitLength() * 
	boost::lexical_cast<Iflt>(XML.getAttribute("Diameter"));
      
      e = boost::lexical_cast<Iflt>(XML.getAttribute("Elasticity"));
      
      d2 = diameter * diameter;
      
      intName = XML.getAttribute("Name");
    }
  catch (boost::bad_lexical_cast &)
    {
      D_throw() << "Failed a lexical cast in IRoughHardSphere";
    }
}

Iflt 
IRoughHardSphere::maxIntDist() const 
{ return diameter; }

Iflt 
IRoughHardSphere::hardCoreDiam() const 
{ return diameter; }

void 
IRoughHardSphere::rescaleLengths(Iflt scale) 
{ 
  diameter += scale*diameter;
  d2 = diameter*diameter;
}

CInteraction* 
IRoughHardSphere::Clone() const 
{ return new IRoughHardSphere(*this); }
  
CIntEvent 
IRoughHardSphere::getEvent(const CParticle &p1, const CParticle &p2) const 
{ 
#ifdef DYNAMO_DEBUG
  if (!Sim->dynamics.getLiouvillean().isUpToDate(p1))
    D_throw() << "Particle 1 is not up to date";
  
  if (!Sim->dynamics.getLiouvillean().isUpToDate(p2))
    D_throw() << "Particle 2 is not up to date";
#endif

#ifdef DYNAMO_DEBUG
  if (p1 == p2)
    D_throw() << "You shouldn't pass p1==p2 events to the interactions!";
#endif 

  CPDData colldat(*Sim, p1, p2);

  if (Sim->dynamics.getLiouvillean().SphereSphereInRoot(colldat, d2))
    {
#ifdef DYNAMO_OverlapTesting
      if (Sim->dynamics.getLiouvillean().sphereOverlap(colldat, d2))
	D_throw() << "Overlapping particles found" 
		  << ", particle1 " << p1.getID() << ", particle2 " 
		  << p2.getID() << "\nOverlap = " << (sqrt(colldat.r2) - sqrt(d2))/Sim->dynamics.units().unitLength();
#endif

      return CIntEvent(p1, p2, colldat.dt, CORE, *this);
    }
  
  return CIntEvent(p1,p2,HUGE_VAL, NONE, *this);  
}

void
IRoughHardSphere::runEvent(const CParticle& p1,
		       const CParticle& p2,
		       const CIntEvent& iEvent) const
{
  ++Sim->lNColl;
    
  //Run the collision and catch the data
  C2ParticleData EDat
    (Sim->dynamics.getLiouvillean().SmoothSpheresColl(iEvent, e, d2)); 

  Sim->signalParticleUpdate(EDat);

  //Now we're past the event, update the scheduler and plugins
  Sim->ptrScheduler->fullUpdate(p1, p2);
  
  BOOST_FOREACH(smrtPlugPtr<OutputPlugin> & Ptr, Sim->outputPlugins)
    Ptr->eventUpdate(iEvent,EDat);
}
   
void 
IRoughHardSphere::outputXML(xmlw::XmlStream& XML) const
{
  XML << xmlw::attr("Type") << "HardSphere"
      << xmlw::attr("Diameter") << diameter / Sim->dynamics.units().unitLength()
      << xmlw::attr("Elasticity") << e
      << xmlw::attr("Name") << intName
      << range;
}

void
IRoughHardSphere::checkOverlaps(const CParticle& part1, const CParticle& part2) const
{
  Vector  rij = part1.getPosition() - part2.getPosition();  
  Sim->dynamics.BCs().applyBC(rij); 
  
  if ((rij | rij) < d2)
    I_cerr() << std::setprecision(std::numeric_limits<float>::digits10)
	     << "Possible overlap occured in diagnostics\n ID1=" << part1.getID() 
	     << ", ID2=" << part2.getID() << "\nR_ij^2=" 
	     << (rij | rij) / pow(Sim->dynamics.units().unitLength(),2)
	     << "\nd^2=" 
	     << d2 / pow(Sim->dynamics.units().unitLength(),2);
}

void 
IRoughHardSphere::write_povray_desc(const DYNAMO::RGB& rgb, const size_t& specID, 
				std::ostream& os) const
{ 
  Iflt locDiam = diameter;

  if (Sim->dynamics.liouvilleanTypeTest<LCompression>())
    locDiam *= 1.0 + static_cast<const LCompression&>(Sim->dynamics.getLiouvillean()).getGrowthRate() * Sim->dSysTime;

  os << "#declare intrep" << ID << " = " 
     << "sphere {\n <0,0,0> " 
     << locDiam / 2.0
    //<< "\n texture { pigment { color rgb<" << rgb.R << "," << rgb.G  << "," << rgb.B 
     << "\n texture { pigment { color rgb<0.8,0.8,0.8" 
     << "> }}\nfinish { phong 0.9 phong_size 60 reflection 0.05 }\n}\n";
  
  BOOST_FOREACH(const size_t& pid, *(Sim->dynamics.getSpecies()[specID]->getRange()))
    {
      Vector  pos(Sim->vParticleList[pid].getPosition());
      Sim->dynamics.BCs().applyBC(pos);

      os << "object {\n intrep" << ID << "\n translate <"
	 << pos[0];
      os << "," << pos[1];
      os << "," << pos[2];      
      os << ">\n}\n";
    }
}

