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

#ifndef CTChains_H
#define CTChains_H

#include "topology.hpp"
#include "../../datatypes/pluginpointer.hpp"
#include "../../base/is_base.hpp"
#include <string>
#include <list>

class CTChain:public CTopology
{
public:  
  CTChain(const XMLNode&, DYNAMO::SimData*, unsigned int ID);

  CTChain(DYNAMO::SimData*, unsigned int ID, std::string);

  virtual ~CTChain() {}
  
  void operator<<(const XMLNode&) {}

  virtual CTChain* Clone() const { return new CTChain(*this); }

protected:
  
  virtual void outputXML(xmlw::XmlStream&) const;
};



#endif
