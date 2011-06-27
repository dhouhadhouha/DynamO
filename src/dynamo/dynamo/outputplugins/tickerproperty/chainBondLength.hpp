/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
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

#pragma once
#include "ticker.hpp"
#include "../../datatypes/histogram.hpp"
#include <boost/shared_array.hpp>

class OPChainBondLength: public OPTicker
{
 public:
  OPChainBondLength(const dynamo::SimData*, const magnet::xml::Node&);

  virtual OutputPlugin *Clone() const
  { return new OPChainBondLength(*this); }

  virtual void initialise();

  virtual void stream(double) {}

  virtual void ticker();

  virtual void changeSystem(OutputPlugin*);

  virtual void output(magnet::xml::XmlStream&);
  
 protected:

  struct Cdata
  {
    Cdata(size_t chainID, size_t CL);
    const size_t chainID;
    std::vector<C1DHistogram> BondLengths;
  };

  std::list<Cdata> chains;

};
