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

#include "include.hpp"
#include "../../datatypes/vector.hpp"
#include <boost/lexical_cast.hpp>
#include "../../extcode/xmlwriter.hpp"
#include "../../extcode/xmlParser.h"
#include "../../base/is_exception.hpp"
#include "../../base/is_simdata.hpp"

xmlw::XmlStream& operator<<(xmlw::XmlStream& XML, 
			    const CUnits& g)
{
  g.outputXML(XML);
  return XML;
}

Iflt 
CUnits::simVolume() const
{ 
  Iflt vol = 1.0;
  for (int iDim = 0; iDim < NDIM; iDim++)
    vol *= Sim->aspectRatio[iDim];
  
  return vol;
}

CUnits* 
CUnits::loadUnits(const XMLNode &XML, const DYNAMO::SimData* Sim)
{
  if (!strcmp(XML.getAttribute("Type"),"Elastic"))
    return new CUElastic(XML, Sim);
  else if (!strcmp(XML.getAttribute("Type"),"Shear"))
    return new CUShear(XML, Sim);
  else if (!strcmp(XML.getAttribute("Type"),"SW"))
    return new CUSW(XML, Sim);
  else
   I_throw() << "Could not recognise the type of the units";
}
