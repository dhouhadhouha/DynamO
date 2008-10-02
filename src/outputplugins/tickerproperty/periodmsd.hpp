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

#ifndef COPPeriodicMSD_H
#define COPPeriodicMSD_H

#include "ticker.hpp"

class COPMSD;

class COPPeriodicMSD: public COPTicker
{
 public:
  COPPeriodicMSD(const DYNAMO::SimData*);

  virtual void initialise();

  void output(xmlw::XmlStream &); 

  virtual COutputPlugin *Clone() const { return new COPPeriodicMSD(*this); };
  
 protected:
  virtual void stream(Iflt) {}  
  virtual void ticker();

  size_t TickerCount;
  
  typedef std::pair<Iflt, Iflt> localpair;

  std::list<localpair> results;

  typedef std::pair<const CTopology*, std::list<localpair> > localpair2;

  std::vector<localpair2> structResults;

  const COPMSD* ptrCOPMSD;
};

#endif
