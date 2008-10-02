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

#ifndef COP2PP_HPP
#define COP2PP_HPP

#include "../outputplugin.hpp"

class COP2PP: public COutputPlugin
{
public:
  COP2PP(const DYNAMO::SimData*, const char*);

  virtual void eventUpdate(const CIntEvent&, const C2ParticleData&);

  virtual void eventUpdate(const CGlobEvent&, const CNParticleData&);

  virtual void eventUpdate(const CSystem&, const CNParticleData&, const Iflt&);

private:
  virtual void A2ParticleChange(const C2ParticleData&) = 0;
  virtual void stream(Iflt) = 0;  
};

#endif
