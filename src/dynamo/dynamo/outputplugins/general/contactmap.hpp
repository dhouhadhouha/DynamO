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
#include <dynamo/outputplugins/outputplugin.hpp>
#include <set>
#include <vector>
#include <unordered_map>

namespace dynamo {
  namespace detail {
    struct OPContactMapHash
    {
      ::std::size_t operator()(const std::vector<std::pair<size_t, size_t> >& map) const;
    };

    struct OPContactMapPairHash
    {
      ::std::size_t operator()(const std::pair<size_t, size_t>& pair) const;
    };
  }
  class OPContactMap: public OutputPlugin
  {
  public:
    OPContactMap(const dynamo::Simulation*, const magnet::xml::Node&);

    ~OPContactMap() {}

    virtual void eventUpdate(const IntEvent&, const PairEventData&);
    virtual void eventUpdate(const GlobalEvent&, const NEventData&);
    virtual void eventUpdate(const LocalEvent&, const NEventData&);
    virtual void eventUpdate(const System&, const NEventData&, const double&);

    virtual void initialise();

    virtual void output(magnet::xml::XmlStream&);

    virtual void operator<<(const magnet::xml::Node&);

    virtual void changeSystem(OutputPlugin*);

  private:
    void stream(double);
    void flush();
    
    double _weight;
    double _total_weight;
    /*! \brief A sorted listing of all the captured pairs in the
     system
    */
    std::set<std::pair<size_t, size_t> > _current_map;
    size_t _next_map_id;

    struct MapData
    {
      MapData(double energy=0, size_t id = 0): _weight(0), _energy(energy), _id(id) {}
      double _weight;
      double _energy;
      size_t _id;
    };

    typedef std::vector<std::pair<size_t, size_t> > MapKey;
    /*! \brief A hash table storing the histogram of the contact maps.
      
      The key of this map is a sorted list of the captured pairs in
      the system. This sorting is implicitly carried out by the
      _current_map.
     */
    std::unordered_map<MapKey, MapData,  detail::OPContactMapHash> _collected_maps;
    std::unordered_map<std::pair<size_t, size_t>, size_t, detail::OPContactMapPairHash> _map_links;
  };
}
