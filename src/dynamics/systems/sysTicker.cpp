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

#include "sysTicker.hpp"
#include "../../base/is_simdata.hpp"
#include "../NparticleEventData.hpp"
#include "../../outputplugins/tickerproperty/ticker.hpp"
#include "../units/units.hpp"

CSTicker::CSTicker(DYNAMO::SimData* nSim, Iflt nPeriod, std::string nName):
  CSystem(nSim)
{
  if (nPeriod <= 0.0)
    nPeriod = Sim->Dynamics.units().unitTime();

  dt = nPeriod;
  period = nPeriod;

  sysName = nName;

  I_cout() << "System ticker set for a peroid of " 
	   << nPeriod / Sim->Dynamics.units().unitTime();
}

void 
CSTicker::stream(Iflt ndt)
{
  dt -= ndt;
}

CNParticleData 
CSTicker::runEvent()
{
  dt += period;

  {
    COPTicker* ptr = NULL;
    BOOST_FOREACH(smrtPlugPtr<COutputPlugin>& Ptr, Sim->outputPlugins)
      {
	ptr = dynamic_cast<COPTicker*>(Ptr.get_ptr());
	if (ptr != NULL)
	  ptr->ticker();
      }
  }

  return CNParticleData();
}

void 
CSTicker::initialise(size_t nID)
{ ID = nID; }

void 
CSTicker::setdt(Iflt ndt)
{ 
  dt = ndt * Sim->Dynamics.units().unitTime(); 
}

void 
CSTicker::increasedt(Iflt ndt)
{ 
  dt += ndt * Sim->Dynamics.units().unitTime(); 
}
