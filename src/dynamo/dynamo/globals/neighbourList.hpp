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
#include <dynamo/globals/global.hpp>
#include <dynamo/simulation.hpp>
#include <dynamo/ranges/IDRangeList.hpp>
#include <boost/function.hpp>
#include <magnet/function/delegate.hpp>
#include <magnet/math/vector.hpp>
#include <memory>
#include <vector>

namespace dynamo {
  /*! \brief A base class for Global events which implement a neighbour list.
    
    This is the interface for neighbour lists, which are used to
    optimise the look up of particles in the neighbourhood of a given
    \ref Particle.
   
    This class also defines callback's that can be registered so that
    other parts of DynamO can be updated when a particle changes
    neighbours.
   */
  class GNeighbourList: public Global
  {
  public:
    /*! \brief The type of function that can be registered for callbacks
     * when new neighbours of a particle have appeared. 
     */
    typedef magnet::function::Delegate2
    <const Particle&, const size_t&, void> nbHoodFunc;

    /*! \brief The type of function that is called back when asking
      for neighbors around a point.
     */
    typedef magnet::function::Delegate1
    <const size_t&, void> nbHoodFunc2;
  
    /*! \brief The type of function that can be registered for callbacks
     * when the neighbourlist is reinitialized.
     */
    typedef magnet::function::Delegate0<void> initFunc;  
  protected:
    typedef std::pair<size_t, nbHoodFunc> nbHoodSlot;

    typedef std::pair<size_t, initFunc> initSlot;

    struct nbHoodSlotEraser
    {
      nbHoodSlotEraser(const size_t& id_in): 
	id(id_in)
      {}

      bool operator()(const nbHoodSlot& nbs) const
      { return  nbs.first == id; } 

      size_t id;
    };

    struct initSlotEraser
    {
      initSlotEraser(const size_t& id_in): 
	id(id_in)
      {}

      bool operator()(const initSlot& nbs) const
      { return  nbs.first == id; } 

      size_t id;
    };

  public:
    GNeighbourList(dynamo::Simulation* a, const char *b): 
      Global(a, b),
      _initialised(false),
      _maxInteractionRange(0),
      isUsedInScheduler(false),
      lambda(0.9)
    {}

    virtual IDRangeList getParticleNeighbours(const Particle&) const = 0;
    virtual IDRangeList getParticleNeighbours(const Vector&) const = 0;

    template<class T> size_t
    ConnectSigCellChangeNotify
    (void (T::*func)(const Particle&, const size_t&)const , const T* tp) const 
    {    
      sigCellChangeNotify.push_back
	(nbHoodSlot(++sigCellChangeNotifyCount, 
		    nbHoodFunc(tp, func)));
    
      return sigCellChangeNotifyCount; 
    }

    inline void
    DisconnectSigCellChangeNotify(const size_t& id) const 
    {    
      sigCellChangeNotify.erase
	(std::remove_if(sigCellChangeNotify.begin(),
			sigCellChangeNotify.end(),
			nbHoodSlotEraser(id)), 
	 sigCellChangeNotify.end());
    }

    template<class T> size_t
    ConnectSigNewNeighbourNotify
    (void (T::*func)(const Particle&, const size_t&) const, const T* tp) const 
    {    
      sigNewNeighbourNotify.push_back
	(nbHoodSlot(++sigNewNeighbourNotifyCount, 
		    nbHoodFunc(tp, func)));
    
      return sigNewNeighbourNotifyCount; 
    }

    inline void
    DisconnectSigNewNeighbourNotify(const size_t& id) const 
    {    
      sigNewNeighbourNotify.erase
	(std::remove_if(sigNewNeighbourNotify.begin(),
			sigNewNeighbourNotify.end(),
			nbHoodSlotEraser(id)), 
	 sigNewNeighbourNotify.end());
    }
    
    template<class T> size_t
    ConnectSigReInitNotify(void (T::*func)(), T* tp) const 
    {    
      sigReInitNotify.push_back
	(initSlot(++sigReInitNotifyCount, 
		  getInitDelegate(func,tp)));
    
      return sigReInitNotifyCount; 
    }

    inline void
    DisconnectSigReInitNotify(const size_t& id) const 
    {    
      sigReInitNotify.erase
	(std::remove_if(sigReInitNotify.begin(),
			sigReInitNotify.end(),
			initSlotEraser(id)), 
	 sigReInitNotify.end());
    }

    /*! \brief This returns the maximum interaction length this
      neighbourlist supports.
      
      Due the to neighbourlists using integer numbers of cells, they
      end up supporting an interaction range larger than 
      \ref getMaxInteractionRange().
    */
    virtual double
    getMaxSupportedInteractionLength() const = 0;

    virtual void reinitialise()
    {
      if (!_maxInteractionRange)
	_maxInteractionRange = Sim->getLongestInteraction();

      _initialised = true;
    }
    
    void markAsUsedInScheduler() { isUsedInScheduler = true; }

    void setCellOverlap(bool overlap) 
    {
      if (overlap)
	lambda = 0.9; 
      else
	lambda = 0.001;
    }

    /*! \brief Set the minimum range this neighbourlist is to support.
      
      This is the minimum as neighbourlists usually must support a
      slightly larger distance.
      
      \sa getMaxSupportedInteractionLength()
     */
    void setMaxInteractionRange(double range)
    {
      _maxInteractionRange = range;
      if (_initialised) reinitialise();
    }

    /*! \brief Returns the requested minimum supported interaction
        range.
     */
    double getMaxInteractionRange() const
    { return _maxInteractionRange; }

  protected:
    bool _initialised;
    double _maxInteractionRange;

    GNeighbourList(const GNeighbourList&);

    virtual void outputXML(magnet::xml::XmlStream&) const = 0;

  
    //Signals
    mutable size_t sigCellChangeNotifyCount;
    mutable std::vector<nbHoodSlot>
    sigCellChangeNotify;

    mutable size_t sigNewNeighbourNotifyCount;
    mutable std::vector<nbHoodSlot>
    sigNewNeighbourNotify;

    mutable size_t sigReInitNotifyCount;
    mutable std::vector<initSlot> 
    sigReInitNotify;

    bool isUsedInScheduler;
    double lambda; 
  };
}

