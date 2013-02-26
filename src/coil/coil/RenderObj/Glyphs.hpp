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

#include <coil/RenderObj/DataSet.hpp>
#include <coil/RenderObj/AttributeColorSelector.hpp>
#include <coil/RenderObj/AttributeOrientationSelector.hpp>
#include <magnet/GL/shader/sphere.hpp>
#include <magnet/GL/shader/simple_render.hpp>
#include <magnet/GL/shader/render.hpp>
#include <magnet/GL/buffer.hpp>

namespace coil {
  class Glyphs : public DataSetChild
  {
    enum GLYPH_TYPE
      {
	SPHERE_GLYPH=0,
	ARROW_GLYPH=1,
	CYLINDER_GLYPH=2,
	LINE_GLYPH=3,
	CUBE_GLYPH=4
      };

  public:
    inline Glyphs(std::string name, DataSet& ds, int initGlyphType = 0): DataSetChild(name, ds), _N(0), _scale(1), _initGlyphType(initGlyphType) {}

    inline ~Glyphs() { deinit(); }

    virtual void glRender(const magnet::GL::Camera&, RenderMode);
    
    virtual void init(const std::shared_ptr<magnet::thread::TaskQueue>&);
    
    virtual void deinit();

    virtual void showControls(Gtk::ScrolledWindow* win);

    virtual Glib::RefPtr<Gdk::Pixbuf> getIcon();

    virtual uint32_t pickableObjectCount()
    { 
      if (visible())
	return _N * (2 * _xperiodicimages->get_value_as_int() + 1)
	  * (2 * _yperiodicimages->get_value_as_int() + 1)
	  * (2 * _zperiodicimages->get_value_as_int() + 1);
	
      return 0; 
    }

    virtual void pickingRender(const magnet::GL::Camera& cam, 
			       const uint32_t offset);

    virtual std::array<GLfloat, 4> getCursorPosition(uint32_t objID);

    virtual std::string getCursorText(uint32_t objID);

    virtual magnet::math::Vector getMaxCoord() const;
    virtual magnet::math::Vector getMinCoord() const;

  protected:
    void glyph_type_changed();
    void guiUpdate();

    virtual magnet::GL::element_type::Enum  getElementType();
    
    std::vector<GLfloat> getPrimitiveVertices();   
    std::vector<GLfloat> getPrimitiveNormals();
    std::vector<GLuint>  getPrimitiveIndicies();

    magnet::GL::Buffer<GLfloat> _primitiveVertices;
    magnet::GL::Buffer<GLfloat> _primitiveNormals;
    magnet::GL::Buffer<GLuint>  _primitiveIndices;

    std::unique_ptr<Gtk::VBox> _gtkOptList;
    std::unique_ptr<AttributeSelector> _scaleSel; 
    std::unique_ptr<AttributeColorSelector> _colorSel;
    std::unique_ptr<AttributeOrientationSelector> _orientSel;
    std::unique_ptr<Gtk::ComboBoxText> _glyphType;
    std::unique_ptr<Gtk::SpinButton> _glyphLOD;
    std::unique_ptr<Gtk::HBox> _glyphBox;
    std::unique_ptr<Gtk::CheckButton> _glyphRaytrace;
    std::unique_ptr<Gtk::SpinButton> _xperiodicimages;
    std::unique_ptr<Gtk::SpinButton> _yperiodicimages;
    std::unique_ptr<Gtk::SpinButton> _zperiodicimages;

    std::unique_ptr<Gtk::HBox>  _scaleFactorBox;
    std::unique_ptr<Gtk::Label> _scaleLabel;
    std::unique_ptr<Gtk::Entry> _scaleFactor;
    
    bool _raytraceable;
    size_t _N;
    float _scale;
    int _initGlyphType;
    magnet::GL::Context::ContextPtr _context;
    magnet::GL::shader::RenderShader _renderShader;
    magnet::GL::shader::SphereShader _sphereShader;
    magnet::GL::shader::SphereVSMShader _sphereVSMShader;
    magnet::GL::shader::SimpleRenderShader _simpleRenderShader;
 };
}
