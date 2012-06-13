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

#pragma once
#include <dynamo/outputplugins/tickerproperty/ticker.hpp>
#include <dynamo/liouvillean/liouvillean.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

namespace dynamo {
  class OPTinkerXYZ: public OPTicker
  {
  public:
    OPTinkerXYZ(const dynamo::SimData*, const magnet::xml::Node&);
    ~OPTinkerXYZ();

    virtual void initialise();

    virtual void stream(double) {}

    virtual void ticker();

    void operator<<(const magnet::xml::Node&);

  protected:
    int frameCount;
    bool fileOutput;
    bool liveOutput;
    bool blockForVMD;
    int max_frame_count;

    bool P1track;

    void *clientsock;
    void *sock;

    int port;

    std::vector<float> coords;

    void printFileImage();
    void printLiveImage();
  };
}
