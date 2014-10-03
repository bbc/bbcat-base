
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "3DPosition.h"

BBC_AUDIOTOOLBOX_START

const Position XAxis(1.0, 0.0, 0.0);
const Position YAxis(0.0, 1.0, 0.0);
const Position ZAxis(0.0, 0.0, 1.0);

/*--------------------------------------------------------------------------------*/
/** Apply rotation
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator *= (const Quaternion& rotation)
{
  *this = *this * rotation;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Remove rotation
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator /= (const Quaternion& rotation)
{
  *this = *this / rotation;
  return *this;
}

/*--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Simple constructor
 *
 * @param phi rotation angle IN DEGREES
 * @param x   x component of axis of rotation
 * @param y   y component of axis of rotation
 * @param z   z component of axis of rotation
 *
 */
/*--------------------------------------------------------------------------------*/
Quaternion::Quaternion(double phi, double _x, double _y, double _z)
{
  Set(phi, _x, _y, _z);
}
Quaternion::Quaternion(double phi, const Position& vec)
{
  Set(phi, vec);
}
Quaternion::Quaternion(const Position& vec)
{
  operator = (vec);
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
Quaternion& Quaternion::operator = (const Quaternion& obj)
{
  w = obj.w;
  x = obj.x;
  y = obj.y;
  z = obj.z;
  return *this; 
}
Quaternion& Quaternion::operator = (const Position& pos)
{
  Position _pos = pos.Cart();
  w = 0.0;
  x = _pos.pos.x;
  y = _pos.pos.y;
  z = _pos.pos.z;
  return *this; 
}

/*--------------------------------------------------------------------------------*/
/** Explicit set functions
 */
/*--------------------------------------------------------------------------------*/
Quaternion& Quaternion::Set(double phi, double _x, double _y, double _z)
{
  // Quaternion version of rotation of phi degrees of (_x, _y, _z) axis is:
  // cos(phi * pi / 360) + (_x.i + _y.j + _z.k).sin(phi * pi / 360)
  double s, m = sqrt(_x * _x + _y * _y + _z * _z); // calculate magnitude of pure vector
  phi *= M_PI / 360.0;                        // convert from degrees to radians and half angle
  w    = cos(phi);
  s    = sin(phi) / ((m > 0.0) ? m : 1.0);    // divide sin multiplier by magnitude of (_x, _y, _z) magnitude to ensure it is a unit vector
  x    = s * _x;
  y    = s * _y;
  z    = s * _z;
  return *this;
}

Quaternion& Quaternion::Set(double phi, const Position& vec)
{
  Position _vec = vec.Cart();
  return Set(phi, _vec.pos.x, _vec.pos.y, _vec.pos.z);
}

/*--------------------------------------------------------------------------------*/
/** Multiply operator - apply second rotation to first
 */
/*--------------------------------------------------------------------------------*/
Quaternion operator * (const Quaternion& obj1, const Quaternion& obj2)
{
  // (a + bi + cj + dk)(e + fi + gi + hk) = 
  //     (ae - bf - cg - dh) + (af + be + ch - dg)i + (ag - bh + ce + df)j + (ah + bg - cf + de)k
  // a = w     e = obj.w
  // b = x     f = obj.x
  // c = y     g = obj.y
  // d = z     h = obj.z
  Quaternion res;
  res.w = obj1.w * obj2.w - obj1.x * obj2.x - obj1.y * obj2.y - obj1.z * obj2.z;
  res.x = obj1.w * obj2.x + obj1.x * obj2.w + obj1.y * obj2.z - obj1.z * obj2.y;
  res.y = obj1.w * obj2.y - obj1.x * obj2.z + obj1.y * obj2.w + obj1.z * obj2.x;
  res.z = obj1.w * obj2.z + obj1.x * obj2.y - obj1.y * obj2.x + obj1.z * obj2.w;
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Divide operator - remove second rotation from first
 */
/*--------------------------------------------------------------------------------*/
Quaternion operator / (const Quaternion& obj1, const Quaternion& obj2)
{
  return obj1 * -obj2;
}

/*--------------------------------------------------------------------------------*/
/** Rotate position by Quaternion
 */
/*--------------------------------------------------------------------------------*/
Position operator * (const Position& pos, const Quaternion& rotation)
{
  // calculate p' = qp(q^-1)
  return ((rotation * Quaternion(pos)) * (-rotation)).GetAxis();
}

/*--------------------------------------------------------------------------------*/
/** Reverse rotate position by Quaternion
 */
/*--------------------------------------------------------------------------------*/
Position operator / (const Position& pos, const Quaternion& rotation)
{
  // calculate p' = (q^-1)pq
  return (((-rotation) * Quaternion(pos)) * rotation).GetAxis();
}

/*--------------------------------------------------------------------------------*/

PositionTransform::PositionTransform()
{
}

PositionTransform::PositionTransform(const PositionTransform& obj)
{
  operator = (obj);
}

PositionTransform::PositionTransform(const Quaternion& obj)
{
  operator = (obj);
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
PositionTransform& PositionTransform::operator = (const PositionTransform& obj)
{
  pretranslation  = obj.pretranslation;
  rotation        = obj.rotation;
  posttranslation = obj.posttranslation;

  return *this;
}

PositionTransform& PositionTransform::operator = (const Quaternion& obj)
{
  pretranslation  = Position();
  rotation        = obj;
  posttranslation = Position();

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Add transform to this transform
 */
/*--------------------------------------------------------------------------------*/
PositionTransform& PositionTransform::operator += (const PositionTransform& obj)
{
  pretranslation  += obj.pretranslation;
  rotation        *= obj.rotation;
  posttranslation += obj.posttranslation;

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Subtract transform to this transform
 */
/*--------------------------------------------------------------------------------*/
PositionTransform& PositionTransform::operator -= (const PositionTransform& obj)
{
  pretranslation  -= obj.pretranslation;
  rotation        /= obj.rotation;
  posttranslation -= obj.posttranslation;

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Apply transform to position
 */
/*--------------------------------------------------------------------------------*/
void PositionTransform::ApplyTransform(Position& pos) const
{
  if (pos.polar)
  {
    // convert to cartesian, transform and then convert back
    pos = pos.Cart();
    ApplyTransform(pos);
    pos = pos.Polar();
  }
  else
  {
    pos += pretranslation;
    pos *= rotation;
    pos += posttranslation;
  }
}

/*--------------------------------------------------------------------------------*/
/** Remove transform to position
 */
/*--------------------------------------------------------------------------------*/
void PositionTransform::RemoveTransform(Position& pos) const
{
  if (pos.polar)
  {
    // convert to cartesian, transform and then convert back
    pos = pos.Cart();
    RemoveTransform(pos);
    pos = pos.Polar();
  }
  else
  {
    pos -= posttranslation;
    pos /= rotation;
    pos -= pretranslation;
  }
}

/*----------------------------------------------------------------------------------------------------*/

ScreenTransform::ScreenTransform() : cx(0.0),
                                     cy(0.0),
                                     sx(1.0),
                                     sy(1.0),
                                     dist(0.0)
{
}

ScreenTransform::ScreenTransform(const ScreenTransform& obj) : cx(obj.cx),
                                                               cy(obj.cy),
                                                               sx(obj.sx),
                                                               sy(obj.sy),
                                                               dist(obj.dist)
{
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
ScreenTransform& ScreenTransform::operator = (const ScreenTransform& obj)
{
  cx   = obj.cx;
  cy   = obj.cy;
  sx   = obj.sx;
  sy   = obj.sy;
  dist = obj.dist;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Apply transform to position
 */
/*--------------------------------------------------------------------------------*/
void ScreenTransform::ApplyTransform(Position& pos) const
{
  if (pos.polar)
  {
    // convert to cartesian, transform and then convert back
    pos = pos.Cart();
    ApplyTransform(pos);
    pos = pos.Polar();
  }
  else
  {
    double m = GetDistanceScale(pos.pos.z);
    
    pos.pos.x = cx + sx * m * pos.pos.x;
    pos.pos.y = cy + sy * m * pos.pos.y;

    // NOTE: pos.pos.z is NOT changed, VERY important for removing transform!
  }
}

/*--------------------------------------------------------------------------------*/
/** Remove transform to position
 */
/*--------------------------------------------------------------------------------*/
void ScreenTransform::RemoveTransform(Position& pos) const
{
  if (pos.polar)
  {
    // convert to cartesian, transform and then convert back
    pos = pos.Cart();
    RemoveTransform(pos);
    pos = pos.Polar();
  }
  else
  {
    double m = GetDistanceScale(pos.pos.z);
    
    pos.pos.x = (pos.pos.x - cx) / (sx * m);
    pos.pos.y = (pos.pos.y - cy) / (sy * m);
  }
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Default constructor, defaults to origin and cartesian co-ordinates
 */
/*--------------------------------------------------------------------------------*/
Position::Position(double x, double y, double z) : polar(false)
{
  memset(&pos, 0, sizeof(pos));
  pos.x = x; pos.y = y; pos.z = z;
}

/*--------------------------------------------------------------------------------*/
/** Return the same position but as polar co-ordinates
 */
/*--------------------------------------------------------------------------------*/
Position Position::Polar() const
{
  Position newpos = *this;
        
  if (!polar)
  {
    newpos.polar = true;
    newpos.pos.d = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
        
    if (newpos.pos.d > 0.0)
    {
      // el = asin(z)
      // az = atan(-x / y)

      double x = pos.x / newpos.pos.d;
      double y = pos.y / newpos.pos.d;
      double z = pos.z / newpos.pos.d;

      // since z = sin(el), el = asin(z)
      newpos.pos.el = asin(z) * 180.0 / M_PI;
      // since x = -sin(az) * cos(el)
      //   and y =  cos(az) * cos(el)
      //     x/y = -sin(az) / cos(az) = -tan(az)
      // =>   az = atan(-x / y) = atan2(-x, y)
      if ((x != 0.0) || (y != 0.0))
      {
        // can do atan2
        newpos.pos.az = atan2(-x, y) * 180.0 / M_PI;
      }
    }
  }
        
  return newpos;
}

/*--------------------------------------------------------------------------------*/
/** Return the same position but as cartesian co-ordinates
 */
/*--------------------------------------------------------------------------------*/
Position Position::Cart() const
{
  Position newpos = *this;
        
  if (polar)
  {
    // x = -sin(az) * cos(el)
    // y =  cos(az) * cos(el)
    // z =  sin(el)

    newpos.polar = false;
    newpos.pos.x = pos.d * -sin(pos.az * M_PI / 180.0) * cos(pos.el * M_PI / 180.0);
    newpos.pos.y = pos.d *  cos(pos.az * M_PI / 180.0) * cos(pos.el * M_PI / 180.0);
    newpos.pos.z = pos.d *  sin(pos.el * M_PI / 180.0);
  }
        
  return newpos;
}

/*--------------------------------------------------------------------------------*/
/** Limit azimuth and elevation
 */
/*--------------------------------------------------------------------------------*/
void Position::LimitAngles()
{
  // ensure azimuth is within limits
  pos.az = fmod(pos.az, 360.0);
  if (pos.az <    0.0) pos.az += 360.0;
  if (pos.az >= 360.0) pos.az -= 360.0;

  // ensure elevation is within limits
  pos.el = fmod(pos.el, 180.0);
  if (pos.el <  -90.0) pos.el += 180.0;
  if (pos.el >=  90.0) pos.el -= 180.0;
}

/*--------------------------------------------------------------------------------*/
/** Translate the current position by the supplied position in cartesian space
 *
 * @note the object remains in the same co-ordinate system as it was
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator += (const Position& obj)
{
  if (polar || obj.polar)
  {
    // translating polar co-ordinates requires changing both to cartesian, translating
    // and then converting this object back to polar
    Position pos = Cart() + obj.Cart();
    *this = polar ? pos.Polar() : pos;
  }
  else
  {
    pos.x += obj.pos.x;
    pos.y += obj.pos.y;
    pos.z += obj.pos.z;
  }

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Translate the current position by the supplied position in cartesian space
 *
 * @note the object remains in the same co-ordinate system as it was
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator -= (const Position& obj)
{
  if (polar || obj.polar)
  {
    // translating polar co-ordinates requires changing both to cartesian, translating
    // and then converting this object back to polar
    Position pos = Cart() - obj.Cart();
    *this = polar ? pos.Polar() : pos;
  }
  else
  {
    pos.x -= obj.pos.x;
    pos.y -= obj.pos.y;
    pos.z -= obj.pos.z;
  }

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Scale the current position
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator *= (double val)
{
  if (polar) pos.d *= val;
  else
  {
    pos.x *= val;
    pos.y *= val;
    pos.z *= val;
  }
  return *this;
}

Position& Position::operator *= (const double vals[3])
{
  if (polar)
  {
    // translating polar co-ordinates requires changing both to cartesian, translating
    // and then converting this object back to polar
    *this = Cart();

    // perform translation and convert back
    *this *= vals;

    *this = Polar();
  }
  else
  {
    pos.x *= vals[0];
    pos.y *= vals[1];
    pos.z *= vals[2];
  }

  return *this;
}

Position& Position::operator *= (const double vals[3][3])
{
  if (polar)
  {
    // translating polar co-ordinates requires changing both to cartesian, translating
    // and then converting this object back to polar
    *this = Cart();

    // perform translation and convert back
    *this *= vals;

    *this = Polar();
  }
  else
  {
    double x = pos.x * vals[0][0] + pos.y * vals[0][1] + pos.z * vals[0][2];
    double y = pos.x * vals[1][0] + pos.y * vals[1][1] + pos.z * vals[1][2];
    double z = pos.x * vals[2][0] + pos.y * vals[2][1] + pos.z * vals[2][2];

    pos.x = x;
    pos.y = y;
    pos.z = z;
  }

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Comparison operators
 */
/*--------------------------------------------------------------------------------*/
bool operator == (const Position& obj1, const Position& obj2)
{
  // try comparing positions as cartesian first
  Position pos1 = obj1.Cart();
  Position pos2 = obj2.Cart();

  if ((pos1.pos.x == pos2.pos.x) &&
      (pos1.pos.y == pos2.pos.y) &&
      (pos1.pos.z == pos2.pos.z)) return true;

  // if that fails, try comparing them as polar
  pos1 = obj1.Polar();
  pos2 = obj2.Polar();

  if ((pos1.pos.az == pos2.pos.az) &&
      (pos1.pos.el == pos2.pos.el) &&
      (pos1.pos.d  == pos2.pos.d)) return true;

  return false;
}

/*--------------------------------------------------------------------------------*/
/** Apply position transform
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator *= (const PositionTransform& trans)
{
  trans.ApplyTransform(*this);
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Apply position transform
 */
/*--------------------------------------------------------------------------------*/
Position operator * (const Position& pos, const PositionTransform& trans)
{
  Position res = pos;
  trans.ApplyTransform(res);
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Remove position transform
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator /= (const PositionTransform& trans)
{
  trans.RemoveTransform(*this);
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Remove position transform
 */
/*--------------------------------------------------------------------------------*/
Position operator / (const Position& pos, const PositionTransform& trans)
{
  Position res = pos;
  trans.RemoveTransform(res);
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Apply screen transform
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator *= (const ScreenTransform& trans)
{
  trans.ApplyTransform(*this);
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Apply screen transform
 */
/*--------------------------------------------------------------------------------*/
Position operator * (const Position& pos, const ScreenTransform& trans)
{
  Position res = pos;
  trans.ApplyTransform(res);
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Remove screen transform
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator /= (const ScreenTransform& trans)
{
  trans.RemoveTransform(*this);
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Remove screen transform
 */
/*--------------------------------------------------------------------------------*/
Position operator / (const Position& pos, const ScreenTransform& trans)
{
  Position res = pos;
  trans.RemoveTransform(res);
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Return antipodean version of position
 */
/*--------------------------------------------------------------------------------*/
Position Position::operator - () const
{
  Position res = *this;

  if (res.polar)
  {
    // move azimuth by 180 degrees (wrapping if necessary)
    res.pos.az += 180.0;
    if (res.pos.az >= 180.0) res.pos.az -= 360.0;

    // flip elevation
    res.pos.el  = -res.pos.el;
  }
  else
  {
    res.pos.x = -res.pos.x;
    res.pos.y = -res.pos.y;
    res.pos.z = -res.pos.z;
  }

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Return unit vector version of this Position
 */
/*--------------------------------------------------------------------------------*/
Position Position::Unit() const
{
  Position pos1 = *this;
  double   d    = pos1.Mod();

  if (d > 0.0) pos1 *= 1.0 / d;

  return pos1;
}

/*--------------------------------------------------------------------------------*/
/** Return dot product of two positions
 */
/*--------------------------------------------------------------------------------*/
double DotProduct(const Position& obj1, const Position& obj2)
{
  Position pos1 = obj1.Cart();      // must be cartesian!
  Position pos2 = obj2.Cart();      // must be cartesian!

  return pos1.pos.x * pos2.pos.x + pos1.pos.y * pos2.pos.y + pos1.pos.z * pos2.pos.z;
}

double DotProduct(const Position& obj1, const double vals[3])
{
  Position pos1 = obj1.Cart();      // must be cartesian!

  return pos1.pos.x * vals[0] + pos1.pos.y * vals[1] + pos1.pos.z * vals[2];
}

/*--------------------------------------------------------------------------------*/
/** Return cross product of two positions
 */
/*--------------------------------------------------------------------------------*/
Position CrossProduct(const Position& obj1, const Position& obj2)
{
  return Position(+(obj1.pos.y * obj2.pos.z - obj2.pos.y * obj1.pos.z),
                  -(obj1.pos.x * obj2.pos.z - obj2.pos.x * obj1.pos.z),
                  +(obj1.pos.x * obj2.pos.y - obj2.pos.x * obj1.pos.y));
}

/*--------------------------------------------------------------------------------*/
/** Return angle between two vectors
 */
/*--------------------------------------------------------------------------------*/
double Angle(const Position& obj1, const Position& obj2)
{
  Position pos1 = obj1.Unit();
  Position pos2 = obj2.Unit();
  double   dot  = DotProduct(pos1, obj2.Unit());
  if ((dot < -1.01) || (dot > 1.01))
  {
    ERROR("Dot product of (%0.3lf, %0.3lf, %0.3lf) and (%0.3lf, %0.3lf, %0.3lf) = %0.6lf",
          pos1.pos.x, pos1.pos.y, pos1.pos.z,
          pos2.pos.x, pos2.pos.y, pos2.pos.z,
          dot);
  }
  dot = LIMIT(dot, -1.0, 1.0);
  return acos(dot) * 180.0 / M_PI;
}

std::string Position::ToString() const
{
  std::string str;

  if (polar)
  {
    Printf(str, "polar (%0.3lf, %0.3lf) x %0.3lfm", pos.az, pos.el, pos.d);
  }
  else
  {
    Printf(str, "cart (%0.3lfm, %0.3lfm, %0.3lfm)", pos.x, pos.y, pos.z);
  }
    
  return str;
}

BBC_AUDIOTOOLBOX_END
