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
#include <magnet/math/matrix.hpp>
#include <array>

namespace magnet {
  namespace GL {
    /*! \brief A 4x4 matrix class for projection/model view matrix
     * math.
     */
    class GLMatrix : public std::array<GLfloat, 4 * 4>
    {
      typedef std::array<GLfloat, 4 * 4> Base;
    public:
      /*! \brief Default constructor.
       */
      GLMatrix() {}

      /*! \brief Copy constructor.
       */
      GLMatrix(const Base& o):
	Base(o)
      {}

      /*! \brief Assignment operator.
       */
      GLMatrix& operator=(const Base& o)
      { Base::operator=(o); return *this; }

      operator math::Matrix() const
      {
	math::Matrix retval;
	for (size_t i(0); i < 3; ++i)
	  for (size_t j(0); j < 3; ++j)
	    retval(i, j) = (*this)[4 * j + i];
	return retval;
      }

      /*! \brief Constructs the matrix from a 3x3 rotation
       * matrix.
       */       
      GLMatrix(const math::Matrix& m)
      {
	Base val =
	  {{GLfloat(m(0,0)), GLfloat(m(1,0)), GLfloat(m(2,0)), 0,
	    GLfloat(m(0,1)), GLfloat(m(1,1)), GLfloat(m(2,1)), 0,
	    GLfloat(m(0,2)), GLfloat(m(1,2)), GLfloat(m(2,2)), 0,
	         0,      0,      0, 1}};
	operator=(val);
      }

      /*! \brief Return an identity matrix.
       */
      inline static GLMatrix identity()
      {
	Base retval = {{1,0,0,0, 
			0,1,0,0,
			0,0,1,0,
			0,0,0,1}};
	return retval;
      }
      
      /*! \brief Return a matrix corresponding to a translation.
       *
       * This command emulates the glTranslate command.
       */
      inline static GLMatrix translate(const math::Vector& vec)
      { return translate(vec[0], vec[1], vec[2]); }

      /*! \brief Return a matrix corresponding to a translation.
       *
       * This command emulates the glTranslate command.
       */
      inline static GLMatrix translate(GLfloat x, GLfloat y, GLfloat z)
      { 
	Base retval = {{1,0,0,0, 
			0,1,0,0,
			0,0,1,0,
			x,y,z,1}};
	return retval;
      }

      /*! \brief Return a matrix corresponding to a scaling.
       *
       * This command emulates the glScale command.
       */
      inline static GLMatrix scale(const math::Vector& vec)
      { return scale(vec[0], vec[1], vec[2]); }

      /*! \brief Return a matrix corresponding to a scaling.
       *
       * This command emulates the glScale command.
       */
      inline static GLMatrix scale(GLfloat x, GLfloat y, GLfloat z)
      {
	Base retval = {{x,0,0,0, 
			0,y,0,0,
			0,0,z,0,
			0,0,0,1}};
	return retval;
      }

      /*! \brief Return a matrix corresponding to a rotation.
       *
       * This command emulates the glRotate command.
       *
       * \param angle The angle of rotation (in degrees).
       * \param axis The axis of rotation.
       */
      inline static GLMatrix rotate(const GLfloat& angle, const math::Vector& axis)
      { return GLMatrix(Rodrigues((angle * M_PI / 180.0f) * axis)); }

      /*! \brief Return a matrix corresponding to a frustrum projection.
       
        This command emulates the glFrustrum command with one
        important exception. There is an additional factor called
        zoffset, which biases all surfaces towards (positive) or away
        (negative) from the camera. This is used to solve Z-fighting
        errors. The resource which explains this value is given here
	http://www.terathon.com/gdc07_lengyel.pdf
	
	If you wish to bias a light source's projection matrix (for
	shadow map calculations) you should set zoffset to 4.8e-7.
       */
      inline static GLMatrix frustrum(const GLfloat left, const GLfloat right, 
				      const GLfloat bottom, const GLfloat top, 
				      const GLfloat nearVal, const GLfloat farVal,
				      const GLfloat zoffset = 0)
      { 
	GLfloat A = (right + left) / (right - left);
	GLfloat B = (top + bottom) / (top - bottom);
	GLfloat C = -(farVal + nearVal) / (farVal - nearVal);
	GLfloat D = -2 * farVal * nearVal / (farVal - nearVal);
	
	Base retval = {{2 * nearVal / (right - left), 0, 0, 0,
			0, 2 * nearVal / (top - bottom), 0, 0,
			A, B, C - zoffset, -1,
			0, 0, D, 0}};
	return retval;
      }

      /*! \brief Return a matrix corresponding to a perspective projection.
       *
       * This command emulates the gluPerspective command.
       */
      inline static GLMatrix perspective(const GLfloat fovy, const GLfloat aspect, 
					 const GLfloat zNear, const GLfloat zFar)
      { 
	GLfloat f = 1 / std::tan(fovy * 0.5f);
	Base retval = {{f / aspect, 0, 0, 0,
			0, f, 0, 0,
			0, 0, (zFar + zNear) / (zNear - zFar), -1,
			0, 0, 2 * zFar * zNear / (zNear - zFar), 0}};
	return retval;
      }

      /*! \brief In-place matrix multiplication.
       */       
      inline GLMatrix operator*=(const GLMatrix& om)
      {
	(*this) = (*this) * om; 
	return *this;
      }

      /*! \brief math::Matrix multiplication operator.
       */       
      inline GLMatrix operator*(const GLMatrix& om) const
      {
	GLMatrix retval;
	for (size_t i(0); i < 4; ++i)
	  for (size_t j(0); j < 4; ++j)
	    {
	      GLfloat sum(0);
	      for (size_t k(0); k < 4; ++k)
		sum += operator[](k * 4 + j) * om[i * 4 + k];
	      retval[i * 4 + j] = sum;
	    }
	return retval;
      }

      /*! \brief Matrix multiplication operator.
       */       
      inline std::array<GLfloat, 4> operator*(const std::array<GLfloat, 4>& vec) const
      {
	std::array<GLfloat, 4> retval = {{0,0,0,0}};
	for (size_t i(0); i < 4; ++i)
	  for (size_t j(0); j < 4; ++j)
	    retval[i] += operator[](j * 4 + i) * vec[j];

	return retval;
      }

      
      /*! \brief Transpose the matrix.
       */       
      inline void transpose()
      {
	for (size_t i(0); i < 4; ++i)
	  for (size_t j(i+1); j < 4; ++j)
	    std::swap((*this)[4*i+j], (*this)[4*j+i]);
      }


      /*! \brief Calculate the inverse of the matrix.
       */       
      GLMatrix inverse() const
      {
	GLMatrix result;

	GLfloat tmp[12];											//temporary pair storage

	//calculate pairs for first 8 elements (cofactors)
	tmp[0] = (*this)[10] * (*this)[15];
	tmp[1] = (*this)[11] * (*this)[14];
	tmp[2] = (*this)[9] * (*this)[15];
	tmp[3] = (*this)[11] * (*this)[13];
	tmp[4] = (*this)[9] * (*this)[14];
	tmp[5] = (*this)[10] * (*this)[13];
	tmp[6] = (*this)[8] * (*this)[15];
	tmp[7] = (*this)[11] * (*this)[12];
	tmp[8] = (*this)[8] * (*this)[14];
	tmp[9] = (*this)[10] * (*this)[12];
	tmp[10] = (*this)[8] * (*this)[13];
	tmp[11] = (*this)[9] * (*this)[12];

	//calculate first 8 elements (cofactors)
	result[0] = tmp[0]*(*this)[5] + tmp[3]*(*this)[6] + tmp[4]*(*this)[7]-tmp[1]*(*this)[5] - tmp[2]*(*this)[6] - tmp[5]*(*this)[7];
	result[1] = tmp[1]*(*this)[4] + tmp[6]*(*this)[6] + tmp[9]*(*this)[7]-tmp[0]*(*this)[4] - tmp[7]*(*this)[6] - tmp[8]*(*this)[7];
	result[2] = tmp[2]*(*this)[4] + tmp[7]*(*this)[5] + tmp[10]*(*this)[7]-tmp[3]*(*this)[4] - tmp[6]*(*this)[5] - tmp[11]*(*this)[7];
	result[3] = tmp[5]*(*this)[4] + tmp[8]*(*this)[5] + tmp[11]*(*this)[6]-tmp[4]*(*this)[4] - tmp[9]*(*this)[5] - tmp[10]*(*this)[6];
	result[4] = tmp[1]*(*this)[1] + tmp[2]*(*this)[2] + tmp[5]*(*this)[3]-tmp[0]*(*this)[1] - tmp[3]*(*this)[2] - tmp[4]*(*this)[3];
	result[5] = tmp[0]*(*this)[0] + tmp[7]*(*this)[2] + tmp[8]*(*this)[3]-tmp[1]*(*this)[0] - tmp[6]*(*this)[2] - tmp[9]*(*this)[3];
	result[6] = tmp[3]*(*this)[0] + tmp[6]*(*this)[1] + tmp[11]*(*this)[3]-tmp[2]*(*this)[0] - tmp[7]*(*this)[1] - tmp[10]*(*this)[3];
	result[7] = tmp[4]*(*this)[0] + tmp[9]*(*this)[1] + tmp[10]*(*this)[2]-tmp[5]*(*this)[0] - tmp[8]*(*this)[1] - tmp[11]*(*this)[2];

	//calculate pairs for second 8 elements (cofactors)
	tmp[0] = (*this)[2]*(*this)[7];
	tmp[1] = (*this)[3]*(*this)[6];
	tmp[2] = (*this)[1]*(*this)[7];
	tmp[3] = (*this)[3]*(*this)[5];
	tmp[4] = (*this)[1]*(*this)[6];
	tmp[5] = (*this)[2]*(*this)[5];
	tmp[6] = (*this)[0]*(*this)[7];
	tmp[7] = (*this)[3]*(*this)[4];
	tmp[8] = (*this)[0]*(*this)[6];
	tmp[9] = (*this)[2]*(*this)[4];
	tmp[10] = (*this)[0]*(*this)[5];
	tmp[11] = (*this)[1]*(*this)[4];

	//calculate second 8 elements (cofactors)
	result[8 ] = tmp[0]*(*this)[13] + tmp[3]*(*this)[14] + tmp[4]*(*this)[15]-tmp[1]*(*this)[13] - tmp[2]*(*this)[14] - tmp[5]*(*this)[15];
	result[9 ] = tmp[1]*(*this)[12] + tmp[6]*(*this)[14] + tmp[9]*(*this)[15]-tmp[0]*(*this)[12] - tmp[7]*(*this)[14] - tmp[8]*(*this)[15];
	result[10] = tmp[2]*(*this)[12] + tmp[7]*(*this)[13] + tmp[10]*(*this)[15]-tmp[3]*(*this)[12] - tmp[6]*(*this)[13] - tmp[11]*(*this)[15];
	result[11] = tmp[5]*(*this)[12] + tmp[8]*(*this)[13] + tmp[11]*(*this)[14]-tmp[4]*(*this)[12] - tmp[9]*(*this)[13] - tmp[10]*(*this)[14];
	result[12] = tmp[2]*(*this)[10] + tmp[5]*(*this)[11] + tmp[1]*(*this)[9]-tmp[4]*(*this)[11] - tmp[0]*(*this)[9] - tmp[3]*(*this)[10];
	result[13] = tmp[8]*(*this)[11] + tmp[0]*(*this)[8] + tmp[7]*(*this)[10]-tmp[6]*(*this)[10] - tmp[9]*(*this)[11] - tmp[1]*(*this)[8];
	result[14] = tmp[6]*(*this)[9] + tmp[11]*(*this)[11] + tmp[3]*(*this)[8]-tmp[10]*(*this)[11] - tmp[2]*(*this)[8] - tmp[7]*(*this)[9];
	result[15] = tmp[10]*(*this)[10] + tmp[4]*(*this)[8] + tmp[9]*(*this)[9]-tmp[8]*(*this)[9] - tmp[11]*(*this)[10] - tmp[5]*(*this)[8];

	// calculate determinant
	GLfloat det = (*this)[0]*result[0]+(*this)[1]*result[1]+(*this)[2]*result[2]
	  +(*this)[3]*result[3];

	for (size_t i(0); i < 4*4; ++i)
	  result[i] /= det;

	if(det==0.0f)
	  for (size_t i(0); i < 4; ++i)
	    for (size_t j(0); j < 4; ++j)
	      result[4*i+j] = (i==j) ? 1 : 0;

	//Now we need to transpose the matrix as we copy out
	result.transpose();
	return result;
      }
    };
  }
}
