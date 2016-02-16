/*
* Author: Sebastian Boettcher
* Small quaternion class with some basic functions
* to be expanded as needed
*
*/

#ifndef quaternion_h
#define quaternion_h

#include <vector>
#include <math.h>


template <typename T>
class quaternion {
 public:
  quaternion() : m_x(0.0), m_y(0.0), m_z(0.0), m_w(1.0) {}
  quaternion(T x, T y, T z, T w) : m_x(x), m_y(y), m_z(z), m_w(w) {normalize();}
  quaternion(T roll, T pitch, T yaw);

  // normalizes the quaternion
  quaternion& normalize();

  // set the quaternion
  quaternion& set(T x, T y, T z, T w);
  // set the quaternion from euler angles
  quaternion& set(T roll, T pitch, T yaw);

  // get the euler angles corresponding to the quaternions rotation
  void toEuler(T &roll, T &pitch, T &yaw);

  T m_x;
  T m_y;
  T m_z;
  T m_w;

};

// constructor from euler angles
template <typename T>
inline quaternion<T>::quaternion(T roll, T pitch, T yaw)
{
  set(roll, pitch, yaw);
}


// normalizes the quaternion
template <typename T>
inline quaternion<T>& quaternion<T>::normalize()
{
  T norm = sqrt(m_x*m_x + m_y*m_y + m_z*m_z + m_w*m_w);

  if (norm == 1.0)
    return  *this;

  m_x /= norm;
  m_y /= norm;
  m_z /= norm;
  m_w /= norm;

  return *this;
}


// set the quaternion
template <typename T>
inline quaternion<T>& quaternion<T>::set(T x, T y, T z, T w)
{
  m_x = x;
  m_y = y;
  m_z = z;
  m_w = w;
  return normalize();
}


// set the quaternion from euler angles
template <typename T>
inline quaternion<T>& quaternion<T>::set(T roll, T pitch, T yaw)
{
  const T sr = sin(roll * 0.5);
  const T sp = sin(pitch * 0.5);
  const T sy = sin(yaw * 0.5);

  const T cr = cos(roll * 0.5);
  const T cp = cos(pitch * 0.5);
  const T cy = cos(yaw * 0.5);

  m_x = sr*cp*cy - cr*sp*sy;
  m_y = cr*sp*cy + sr*cp*sy;
  m_z = cr*cp*sy - sr*sp*cy;
  m_w = cr*cp*cy + sr*sp*sy;

  return normalize();
}


// get the euler angles corresponding to the quaternions rotation
template <typename T>
inline void quaternion<T>::toEuler(T &roll, T &pitch, T &yaw)
{
  T test = 2.0 * (m_y*m_w - m_x*m_z);

  if (test > 0.999) { // north pole singularity
    roll = 0.0;
    pitch = M_PI/2.0;
    yaw = -2.0*atan2(m_x, m_w);
  } else if (test < -0.999) { // south pole singularity
    roll = 0.0;
    pitch = -M_PI/2.0;
    yaw = 2.0*atan2(m_x, m_w);
  } else {
    const T sqx = m_x*m_x;
    const T sqy = m_y*m_y;
    const T sqz = m_z*m_z;
    const T sqw = m_w*m_w;
    roll = atan2(2.0*(m_y*m_z + m_x*m_w), -sqx-sqy+sqz+sqw);
    pitch = asin(test);
    yaw = atan2(2.0*(m_x*m_y+m_z*m_w), sqx-sqy-sqz+sqw);
  }
}

#endif // quaternion_h

