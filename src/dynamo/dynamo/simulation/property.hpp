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
#include <magnet/exception.hpp>
#include <magnet/xmlwriter.hpp>
#include <magnet/xmlreader.hpp>
#include <magnet/thread/refPtr.hpp>
#include <magnet/units.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

//! \brief A interface class which allows other classes to access a property
//! of a particle.  
//!
//! These properties are looked up by a name, and the value extracted
//! using the ID of a particle. Some properties are just a single
//! fixed value, their name is their value (see
//! NumericProperty). Others are more complicated and use look-up
//! tables or functions. These are usually defined in the
//! PropertyStore and PropertyHandles are used to access them.
class Property
{
public:
  typedef magnet::units::Units Units;

  inline Property(Units units): _units(units) {}

  //! Fetch the value of this property for a particle with a certain ID
  inline virtual const double& getProperty(size_t ID) const 
  { M_throw() << "Unimplemented"; }

  //! Fetch the maximum value of this property
  inline virtual const double& getMaxValue() const
  { M_throw() << "Unimplemented"; }

  //! This is called whenever a unit is rescaled.
  //!
  //! This function must check the _units of the property and raise
  //! the rescale factor to the correct power.
  //! \param dim The unit that is being rescaled [(L)ength, (T)ime, (M)ass].
  //! \param rescale The factor to rescale the unit by.
  inline virtual const void rescaleUnit(const Units::Dimension dim, 
					const double rescale)
  { M_throw() << "Unimplemented"; }
  
  //! Fetch the name of this property
  inline virtual std::string getName() const 
  { M_throw() << "Unimplemented"; }

  //! Fetch the units of this property
  inline const Units& getUnits() const { return _units; }

  //! Helper to write out derived classes
  friend magnet::xml::XmlStream operator<<(magnet::xml::XmlStream& XML, const Property& prop)
  { prop.outputXML(XML); return XML; }

  //! Write any XML attributes that store this Property's data on a
  //! single particle.
  //! \param pID The ID number of the particle being written out.
  //! \param rescale Amount to scale the Property values by.
  inline virtual void outputParticleXMLData(magnet::xml::XmlStream& XML, 
					    const size_t pID) const {}

protected:
  virtual void outputXML(magnet::xml::XmlStream& XML) const 
  { M_throw() << "Unimplemented"; }

  //! The Units of the property.
  magnet::units::Units _units;
};

//! \brief A class where the name is the value of the property.
//!
//! This property is used whenever a single value is set for a
//! property, e.g., in an interaction the interaction diameter might
//! be R="1.0". A NumericProperty will be generated by the
//! PropertyStore from the value "1.0".
class NumericProperty: public Property
{
public:
  inline NumericProperty(double val, const Property::Units& units):
    Property(units), _val(val) {}
  
  //! Always returns a single value.
  inline virtual const double& getProperty(size_t ID) const { return _val; }
  //! Returns the value as a string.
  inline virtual std::string getName() const { return boost::lexical_cast<std::string>(_val); }

  //! As this Property only stores a single value, it is always
  //! returned as the max.
  inline virtual const double& getMaxValue() const { return _val; }

  //! \sa Property::rescaleUnit
  inline virtual const void rescaleUnit(const Units::Dimension dim, 
					const double rescale)
  { _val *= std::pow(rescale, _units.getUnitsPower(dim));  }

private:
  //! The name of this class is its value. So when other classes
  //! output the name of the property, this counts as outputing the
  //! XML for it. So no extra XML tag is needed.
  virtual void outputXML(magnet::xml::XmlStream& XML) const {}

  //! \brief The single value stored in the property
  double _val;
};

//! \brief A class which stores a single value for each particle.
//!
//! This is the second most common property after NumericProperty. It
//! stores a single float per particle, allowing polydisperse values
//! to be used in the simulation.
class ParticleProperty: public Property
{
public:
  inline ParticleProperty(size_t N, 
			  const Property::Units& units, 
			  std::string name,
			  double initalval):
    Property(units), _name(name),
    _values(N, initalval) {}
  
  inline ParticleProperty(const magnet::xml::Node& node):
    Property(Property::Units(node.getAttribute("Units").getValue())),
    _name(node.getAttribute("Name").getValue())
  {
    //Move up to the particles nodes, and start loading the property values
    for (magnet::xml::Node pNode = node.getParent().getParent()
	   .getNode("ParticleData").getNode("Pt");
	 pNode.valid(); ++pNode)
      _values.push_back(pNode.getAttribute(_name).as<double>());
  }
  
  inline virtual const double& getProperty(size_t ID) const 
  { 
#ifdef DYNAMO_DEBUG
    if (ID >= _values.size())
      M_throw() << "Out of bounds access to ParticleProperty \"" 
		<< _name << "\", which has " << _values.size() 
		<< " entries and you're accessing " << ID;
#endif
    return _values[ID]; 
  }

  inline virtual double& getProperty(size_t ID)
  { 
#ifdef DYNAMO_DEBUG
    return _values.at(ID); 
#endif
    return _values[ID]; 
  }
  
  inline virtual std::string getName() const 
  { return _name; }
  
  inline virtual const double& getMaxValue() const 
  { return *std::max_element(_values.begin(), _values.end()); }
  
  //! \sa Property::rescaleUnit
  inline virtual const void rescaleUnit(const Units::Dimension dim, 
					const double rescale)
  {
    double factor = std::pow(rescale, _units.getUnitsPower(dim));
    if (factor)
      for (Iterator it = _values.begin(); it != _values.end(); ++it)
	*it *= factor;  
  }

  inline void outputParticleXMLData(magnet::xml::XmlStream& XML, const size_t pID) const
  { XML << magnet::xml::attr(_name) << getProperty(pID); }
  
  
protected:
  //! \brief Output an XML representation of the Property to the
  //! passed XmlStream.
  virtual void outputXML(magnet::xml::XmlStream& XML) const 
  { 
    XML << magnet::xml::tag("Property") 
	<< magnet::xml::attr("Type") << "PerParticle"
	<< magnet::xml::attr("Name") << _name
	<< magnet::xml::attr("Units") << std::string(_units)
	<< magnet::xml::endtag("Property");
  }
  
  std::string _name;
  typedef std::vector<double> Container;
  typedef Container::iterator Iterator;
  Container _values;
};

/*! \brief This class stores the properties of the particles loaded from the
 * configuration file and hands out reference counting pointers to the
 * properties to other classes when they're requested by name.
 */
class PropertyStore
{
  typedef magnet::thread::RefPtr<Property> Value;
  typedef std::vector<Value> Container;
  
  //!\brief Contains the NumericProperty's that are defined by their
  //!name.
  //!
  //!These are only stored in the PropertyStore for unit rescaling.
  Container _numericProperties;

  //!Contains the properties that are looked up by their name.
  Container _namedProperties;

  typedef Container::iterator iterator;

public:
  typedef Container::const_iterator const_iterator;

  /*! \brief Request a handle to a property using a string containing
     the properties name.

    If the name is a string representation of a numeric type, the look-up
    in the property store will fail but a one-time NumericProperty is
    created. You may then have lines in the configuration file like so

    For a fixed value
    <Interaction Elasticity="0.9" ... 

    or for a lookup in the property store
    <Interaction Elasticity="e" ... For a lookup of the particle property "e"

    \param name An Attribute containing either the name or the value of a property.
    \return A reference to the property requested or an instance of NumericProperty.
  */
  inline magnet::thread::RefPtr<Property> getProperty(const std::string& name,
						      const Property::Units& units)
  {
    try { return getPropertyBase(name, units); }
    catch (boost::bad_lexical_cast&)
      { M_throw() << "Could not find the property named by " << name; }
  }

  //! \brief Request a handle to a property using an xml attribute
  //!  containing the properties name. 
  //!
  //! See getProperty(const std::string& name) for usage info.
  inline magnet::thread::RefPtr<Property> getProperty(const magnet::xml::Attribute& name,
						      const Property::Units& units)
  {
    try { return getPropertyBase(name.getValue(), units); }
    catch (boost::bad_lexical_cast&)
      { M_throw() << "Could not find the property named by " << name.getPath(); }
  }

  //! \brief Request a handle to a property, but this specialization always
  //!  returns a new instance of NumericProperty.
  //!  \sa getProperty(const std::string& name)
  inline magnet::thread::RefPtr<Property> getProperty(const double& name, 
						      const Property::Units& units)
  {
    Value retval(new NumericProperty(name, units));
    _numericProperties.push_back(retval);
    return retval;
  }

  //! \brief Method which loads the properties from the XML configuration file. 
  //! \param node A xml Node at the root dynamoconfig Node of the config file.
  inline PropertyStore& operator<<(const magnet::xml::Node& node)
  {
    if (node.hasNode("Properties"))
      for (magnet::xml::Node propNode = node.getNode("Properties").fastGetNode("Property");
	   propNode.valid(); ++propNode)
	if (!std::string("PerParticle").compare(propNode.getAttribute("Type")))
	  _namedProperties.push_back(new ParticleProperty(propNode));
	else
	  M_throw() << "Unsupported Property type, " << propNode.getAttribute("Type");
    
    return *this;
  }

  inline friend magnet::xml::XmlStream& operator<<(magnet::xml::XmlStream& XML, const PropertyStore& propStore)
  {
    XML << magnet::xml::tag("Properties");

    for (const_iterator iPtr = propStore._namedProperties.begin(); 
	 iPtr != propStore._namedProperties.end(); ++iPtr)
      XML << (*(*iPtr));

    XML << magnet::xml::endtag("Properties");

    return XML;
  }

  //! \brief Function to rescale the units of all Property-s.
  //!
  //! \param dim The unit that is being rescaled [(L)ength, (T)ime, (M)ass].
  //! \param rescale The factor to rescale the unit by.
  inline const void rescaleUnit(const Property::Units::Dimension dim, 
				const double rescale)
  {  
    for (iterator iPtr = _numericProperties.begin(); 
	 iPtr != _numericProperties.end(); ++iPtr)
      (*iPtr)->rescaleUnit(dim, rescale);

    for (iterator iPtr = _namedProperties.begin(); 
	 iPtr != _namedProperties.end(); ++iPtr)
      (*iPtr)->rescaleUnit(dim, rescale);
  }

  //! \brief Write any XML attributes relevent to Property-s for a single
  //! particle.
  //!
  //! \param pID The ID number of the particle whose data is to be
  //! written out.
  inline void outputParticleXMLData(magnet::xml::XmlStream& XML, size_t pID) const 
  {
    for (const_iterator iPtr = _namedProperties.begin(); 
	 iPtr != _namedProperties.end(); ++iPtr)
      (*iPtr)->outputParticleXMLData(XML, pID);
  }

  /*! \brief Method for pushing constructed properties into the
   * PropertyStore.
   *
   * This method should only be used when dynamod is building a
   * simulation, as the typical method for adding a Property to the
   * PropertyStore is using the \ref getProperty methods.
   */
  inline magnet::thread::RefPtr<Property> push(Property* newProp)
  {
    if (dynamic_cast<NumericProperty*>(newProp))
      {
	_numericProperties.push_back(newProp);
	return _numericProperties.back();
      }    
    
    _namedProperties.push_back(newProp);
    return _namedProperties.back();
  }

protected:

  inline magnet::thread::RefPtr<Property> getPropertyBase(const std::string name,
							  const Property::Units& units)
  {
    //Try name based lookup first
    for (const_iterator iPtr = _namedProperties.begin(); 
	 iPtr != _namedProperties.end(); ++iPtr)
      if ((*iPtr)->getName() == name)
	{
	  if ((*iPtr)->getUnits() == units) return *iPtr;
	  
	  M_throw() << "Property \"" << name << "\" found with units of " 
		    << std::string((*iPtr)->getUnits())
		    << ", but the requested property has units of " 
		    << std::string(units);
	}
    //Try name-is-the-value lookup, if this fails a
    //boost::bad_lexical_cast& will be thrown and must be caught by
    //the caller. 
    return getProperty(boost::lexical_cast<double>(name), units);
  }
};
