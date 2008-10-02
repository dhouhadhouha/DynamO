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

/*! \file Particle.h
\brief Contains the definitions for the Particle class code in Particle.cc
*/

#ifndef SIMULATION_H
#define SIMULATION_H

#include "../base/is_base.hpp"
#include "../base/is_simdata.hpp"
#include "../datatypes/pluginpointer.hpp"
#include "../base/is_exception.hpp"
#include "../dynamics/dynamics.hpp"
#include <boost/scoped_array.hpp>
#include <list>

class CSimImage;
class CDynamics;
class COutputPlugin;
class CStreamTask;

class CSimulation: public DYNAMO::Base_Class, public DYNAMO::SimData
{
 public:
  friend class CReplex;

  CSimulation();

  ~CSimulation();

  void initialise();

  inline void runSilentSimulation() { runSimulation(true); }
  //Gives a function signature to use
  inline void runSimulation() { runSimulation(false); }
  void runSimulation(bool silentMode);
  
  void loadXMLfile(const char *);
  
  void writeXMLfile(const char *);

  void loadPlugins(std::string);

  void outputData(const char* filename = "output.xml.bz2");
  
  void configLoaded();

  void simShutdown();

  void setTrajectoryLength(unsigned long long);

  void setnPrint(unsigned long long);

  void setnImage(unsigned long long);

  void setRandSeed(unsigned int);

  void addGlobal(CGlobal*);

  void addSystem(CSystem*);

  void initPlugins();

  void setSimID(unsigned int n) { simID = n; }

  CSystem* getSystem(std::string);

  long double getSysTime();

  inline const boost::scoped_ptr<DYNAMO::CEnsemble>& getEnsemble() const { return Ensemble; }

  inline boost::scoped_ptr<DYNAMO::CEnsemble>& getEnsemble() { return Ensemble; }
  
  std::ostringstream &getHistory()
    {
      return ssHistory;
    }

  inline const unsigned long long &getnColl() const 
  { return lNColl; }

  inline const ESimulationStatus& getStatus() const
  { return status; }

  //Template member functions must be defined in the header file
  template <class T> T* addOutputPlugin()
    {
      if (status != INITIALISED)
	I_throw() << "Cannot add plugins now";

      try {
	//It's already in the simulation
	return getOutputPlugin<T>();
      } catch (DYNAMO::Exception&)
	{
	  //Its not in the simulation, add it then
	  smrtPlugPtr<COutputPlugin> tempPlug(new T(this));
	  outputPlugins.push_back( tempPlug );
	  return static_cast<T*>(&(*(outputPlugins.back())));
	}

    }

  void addOutputPlugin(std::string);
  
  template <class T> void setPBC()
    {
      Dynamics.setPBC<T>();
    }

  void setTickerPeriod(Iflt);
  
 private:
  void executeEvent();
  void executeIntEvent();
  void executeGlobEvent();
  void executeSysEvent();

  CSimImage makeImage();
  void takeImage();
  void recoverImage();
  void restoreImage(const CSimImage &);
  //void streamTaskInit();

  std::list<CSimImage> simImages;  
  unsigned long long nImage;
  unsigned long long rebuildnColl;
  Iflt localeps;

  //int nStreamTasks;
  //Iflt streamdt;
  //std::list<CStreamTask> streamTasks;
};

#endif
