
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>

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
/** Extract position (either polar or cartesian) from a set of parameters
 */
/*--------------------------------------------------------------------------------*/
bool Position::GetFromParameters(const ParameterSet& parameters, const std::string& name)
{
  ParameterSet subparameters = parameters.GetSubParameters(name);
  bool success;
  bool radians = false;

  polar = false;
  subparameters.Get("polar", polar);
  subparameters.Get("radians", radians);        // allow polar co-ordiniates in either degrees or radians
                                                // the default is DEGREES!

  success = ((!polar &&
              subparameters.Get("x", pos.x) &&
              subparameters.Get("y", pos.y) &&
              subparameters.Get("z", pos.z)) ||
             (polar &&
              subparameters.Get("az", pos.az) &&
              subparameters.Get("el", pos.el) &&
              subparameters.Get("d",  pos.d)));

  // expect polar co-ordinates in radians, convert to degrees
  if (success && polar && radians)
  {
    pos.az *= 180.0 / M_PI;
    pos.el *= 180.0 / M_PI ;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set position (either polar or cartesian) in a set of parameters
 *
 * @note if radians = true, polar values are set in radians
 */
/*--------------------------------------------------------------------------------*/
void Position::SetParameters(ParameterSet& parameters, const std::string& name, bool radians) const
{
  if (polar)
  {
    double mul = (radians ? M_PI / 180.0 : 1.0);

    parameters.Set(name + ".polar", 1);
    if (radians) parameters.Set(name + ".radians", 1);
    else         parameters.Delete(name + ".radians");
    parameters.Set(name + ".az", pos.az * mul);
    parameters.Set(name + ".el", pos.el * mul);
    parameters.Set(name + ".d",  pos.d);
  }
  else
  {
    parameters.Set(name + ".polar", 0);
    parameters.Delete(name + ".radians");
    parameters.Set(name + ".x", pos.x);
    parameters.Set(name + ".y", pos.y);
    parameters.Set(name + ".z", pos.z);
  }
}

/*--------------------------------------------------------------------------------*/

Quaternion::Quaternion(double _w, double _x, double _y, double _z) : w(_w),
                                                                     x(_x),
                                                                     y(_y),
                                                                     z(_z)
{
}

Quaternion::Quaternion(double phi, const Position& vec)
{
  SetFromAngleAxis(phi, vec);
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

Quaternion& Quaternion::operator = (const Position& vec)
{
  Position _vec = vec.Cart();
  w = 0.0;
  x = _vec.pos.x;
  y = _vec.pos.y;
  z = _vec.pos.z;
  return *this; 
}

/*--------------------------------------------------------------------------------*/
/** Comparison operators
 */
/*--------------------------------------------------------------------------------*/
bool operator == (const Quaternion& obj1, const Quaternion& obj2)
{
  return ((obj1.w == obj2.w) && (obj1.x == obj2.x) && (obj1.y == obj2.y) && (obj1.z == obj2.z));
}

/*--------------------------------------------------------------------------------*/
/** Set the Quaternion from raw coeffs
 */
/*--------------------------------------------------------------------------------*/
Quaternion& Quaternion::SetFromCoeffs(double _w, double _x, double _y, double _z)
{
  w = _w;
  x = _x;
  y = _y;
  z = _z;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Set the Quaternion from an axis and an angle (in degrees).
 */
/*--------------------------------------------------------------------------------*/
Quaternion& Quaternion::SetFromAngleAxis(double angle, double _x, double _y, double _z)
{
  // Quaternion version of rotation of phi degrees of (_x, _y, _z) axis is:
  // cos(phi * pi / 360) + (_x.i + _y.j + _z.k).sin(phi * pi / 360)
  double s, m = sqrt(_x * _x + _y * _y + _z * _z);      // calculate magnitude of pure axistor
  angle *= M_PI / 360.0;                                // convert from degrees to radians and half angle
  w = cos(angle);
  s = sin(angle) / ((m > 0.0) ? m : 1.0);               // divide sin multiplier by magnitude of (_x, _y, _z) magnitude to ensure it is a unit axistor
  x = s * _x;
  y = s * _y;
  z = s * _z;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Set the Quaternion from an axis and an angle (in degrees).
 */
/*--------------------------------------------------------------------------------*/
Quaternion& Quaternion::SetFromAngleAxis(double angle, const Position& axis)
{
  Position _axis = axis.Cart();
  return SetFromAngleAxis(angle, _axis.pos.x, _axis.pos.y, _axis.pos.z);
}

/*--------------------------------------------------------------------------------*/
/** Multiply operator - scalar multiply
 */
/*--------------------------------------------------------------------------------*/
Quaternion operator * (const Quaternion& obj1, double val)
{
  Quaternion res;

  res.w = obj1.w * val;
  res.x = obj1.x * val;
  res.y = obj1.y * val;
  res.z = obj1.z * val;

  return res;
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
  return obj1 * obj2.Invert();
}

/*--------------------------------------------------------------------------------*/
/** Rotate position by Quaternion
 */
/*--------------------------------------------------------------------------------*/
Position operator * (const Position& pos, const Quaternion& rotation)
{
  // calculate p' = qp(q^-1)
  return ((rotation * Quaternion(pos)) * rotation.Invert()).GetAxis();
}

/*--------------------------------------------------------------------------------*/
/** Divide operator - scalar divide
 */
/*--------------------------------------------------------------------------------*/
Quaternion operator / (const Quaternion& obj1, double val)
{
  Quaternion res;

  res.w = obj1.w / val;
  res.x = obj1.x / val;
  res.y = obj1.y / val;
  res.z = obj1.z / val;

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Reverse rotate position by Quaternion
 */
/*--------------------------------------------------------------------------------*/
Position operator / (const Position& pos, const Quaternion& rotation)
{
  // calculate p' = (q^-1)pq
  return ((rotation.Invert() * Quaternion(pos)) * rotation).GetAxis();
}

/*--------------------------------------------------------------------------------*/
/** Add a Quaternion to this one
 */
/*--------------------------------------------------------------------------------*/
void Quaternion::operator += (const Quaternion &quaternion)
{
  Quaternion q = *this;
  w = q.w + quaternion.w;
  x = q.x + quaternion.x;
  y = q.y + quaternion.y;
  z = q.z + quaternion.z;
}

/*--------------------------------------------------------------------------------*/
/** Add two Quaternions
 */
/*--------------------------------------------------------------------------------*/
Quaternion operator + (const Quaternion &q0, const Quaternion &q1)
{
  Quaternion q;
  q.w = q0.w + q1.w;
  q.x = q0.x + q1.x;
  q.y = q0.y + q1.y;
  q.z = q0.z + q1.z;
  return q;
}

/*--------------------------------------------------------------------------------*/
/** Subtract a Quaternion from this one
 */
/*--------------------------------------------------------------------------------*/
void Quaternion::operator -= (const Quaternion &quaternion)
{
  Quaternion q = *this;
  w = q.w - quaternion.w;
  x = q.x - quaternion.x;
  y = q.y - quaternion.y;
  z = q.z - quaternion.z;
}

/*--------------------------------------------------------------------------------*/
/** Subtract two Quaternions (q0-q1)
 */
/*--------------------------------------------------------------------------------*/
Quaternion operator - (const Quaternion &q0, const Quaternion &q1)
{
  Quaternion q;
  q.w = q0.w - q1.w;
  q.x = q0.x - q1.x;
  q.y = q0.y - q1.y;
  q.z = q0.z - q1.z;
  return q;
}

/*--------------------------------------------------------------------------------*/
/** Negate the Quaternion
 *
 * @note in rotation terms, the Quaternion still does the same but rotates in the opposite direction
 */
/*--------------------------------------------------------------------------------*/
Quaternion Quaternion::operator - () const
{
  Quaternion q;
  q.w = -w;
  q.x = -x;
  q.y = -y;
  q.z = -z;
  return q;
}

/*--------------------------------------------------------------------------------*/
/** Invert the rotation represented by the Quaternion
 */
/*--------------------------------------------------------------------------------*/
Quaternion Quaternion::Invert() const
{
  Quaternion q;
  q.w =  w;
  q.x = -x;
  q.y = -y;
  q.z = -z;
  return q;
}

/*--------------------------------------------------------------------------------*/
/** Return the scalar (dot) product as generated for normal 4D vectors
 */
/*--------------------------------------------------------------------------------*/
double Quaternion::ScalarProduct(const Quaternion &q) const
{
  return w * q.w + x * q.x + y * q.y + z * q.z;
}

/*--------------------------------------------------------------------------------*/
/** Determine the logarithm of this quaternion (only valid for unit quaternions) : axis*angle
 */
/*--------------------------------------------------------------------------------*/
Quaternion Quaternion::Log() const
{
  double angle = acos(w);
  double sinangle = sin(angle);
  Quaternion q; // q.w =  0.0

  if (sinangle > 0.0) {
      q.x = angle * x / sinangle;
      q.y = angle * y / sinangle;
      q.z = angle * z / sinangle;
  }
  return q;
}

/*--------------------------------------------------------------------------------*/
/** Determine the outcome of using this quaternion as an exponent e^q -
 * note: due to noncommutativity of quaternion multiplication,
 * standard identities for exponential and logarthmic functions do not apply.
 */
/*--------------------------------------------------------------------------------*/
Quaternion Quaternion::Exp() const
{
  double angle = acos(w);
  double sinangle = sin(angle);
  double cosangle = cos(angle);
  Quaternion q;

  q.w = cosangle;
  if (angle > 0.0) {
      q.x = sinangle * x / angle;
      q.y = sinangle * y / angle;
      q.z = sinangle * z / angle;
  }
  return q;
}

/*--------------------------------------------------------------------------------*/
/** Return a normalised version of the quaternion. The ensures that it represents a
 * rotation, can be used to check rounding errors.
 */
/*--------------------------------------------------------------------------------*/
Quaternion Quaternion::Normalised() const
{
  Quaternion q;
  double norm = 1 / sqrt( w*w + x*x + y*y + z*z );
  q.w = w*norm; q.x = x*norm; q.y = y*norm; q.z = z*norm;
  return q;
}

/*--------------------------------------------------------------------------------*/
/** Perform Linear intERPolation between two unit quaternions.
 *
 * @param q0 The initial orientation.
 * @param q1 The final orientation.
 * @param t  A value between 0 and 1 (0 returns q0, 1 returns q1).
 *
 * @note Assumes that quaternions have unit-length i.e. have been normalised.
 * This is to ensure that this method is fast.
 */
/*--------------------------------------------------------------------------------*/
Quaternion Lerp(const Quaternion& q0, const Quaternion& q1, double t)
{
  if (t < 0.0 || t > 1.0)
      BBCERROR("Slerp - t should be between 0 and 1");

  return (q0 * (1.0 - t) + q1 * t);
}

/*--------------------------------------------------------------------------------*/
/** Perform Spherical Linear intERPolation between two unit quaternions. This can be
 * used to achieve an interpolated rotation with constant angular velocity.
 *
 * @param q0 The initial orientation.
 * @param q1 The final orientation.
 * @param t  A value between 0 and 1 (0 returns q0, 1 returns q1).
 *
 * @note Assumes that quaternions have unit-length i.e. have been normalised.
 * This is to ensure that this method is fast.
 */
/*--------------------------------------------------------------------------------*/
Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, double t)
{
  Quaternion q = q1;
  double dot   = q0.ScalarProduct(q1);

  if (t < 0.0 || t > 1.0)
    BBCERROR("Slerp - t should be between 0 and 1");

  // dot = cos(angle) if(dot<0) this & q are more than 90degrees apart
  // we can negate one (still represents the same rotation) but slerp will take the shorter path
  if(dot < 0.0)
  {
    dot = -dot;
    q   = -q;
  }

  if(dot < 0.95)
  {
    // slerp(t;q1,q2) = (q1*sin(theta*(1-t)) + q2*sin(theta*t))/sin(theta) where 0.0<t<1.0
    double angle = acos(dot);
    return (q0 * sin(angle * (1.0 - t)) + q * sin(angle * t)) / sin(angle);
  } else {
    // small angle between them, use linear interpolation
    return Lerp(q0, q1,t);
  }
}


/*--------------------------------------------------------------------------------*/
/** Extract rotation (either angular or pure Quaternion) from a set of parameters
 */
/*--------------------------------------------------------------------------------*/
bool Quaternion::GetFromParameters(const ParameterSet& parameters, const std::string& name)
{
  ParameterSet subparameters = parameters.GetSubParameters(name);
  bool success = false;

  if (subparameters.Get("angle", w) &&
      subparameters.Get("x", x) &&
      subparameters.Get("y", y) &&
      subparameters.Get("z", z))
  {
    SetFromAngleAxis(w, x, y, z);
    success  = true;
  }
  else if (subparameters.Get("w", w) &&
           subparameters.Get("x", x) &&
           subparameters.Get("y", y) &&
           subparameters.Get("z", z))
  {
    success  = true;
  }
  
  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set rotation (as pure Quaternion) in a set of parameters
 */
/*--------------------------------------------------------------------------------*/
void Quaternion::SetParameters(ParameterSet& parameters, const std::string& name) const
{
  parameters.Set(name + ".w", w);
  parameters.Set(name + ".x", x);
  parameters.Set(name + ".y", y);
  parameters.Set(name + ".z", z);
}

/*--------------------------------------------------------------------------------*/
/** Generate friendly text string
 */
/*--------------------------------------------------------------------------------*/
std::string Quaternion::ToString() const
{
  std::string res;
  Printf(res, "%0.14le,%0.14le,%0.14le,%0.14le", w, x, y, z);
  return res;
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
/** Comparison operators
 */
/*--------------------------------------------------------------------------------*/
bool operator == (const PositionTransform& obj1, const PositionTransform& obj2)
{
  return ((obj1.pretranslation  == obj2.pretranslation)  &&
          (obj1.rotation        == obj2.rotation)        &&
          (obj1.posttranslation == obj2.posttranslation));
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
/** Negation operator
 */
/*--------------------------------------------------------------------------------*/
PositionTransform PositionTransform::operator - () const
{
  PositionTransform res;

  res.pretranslation  = -pretranslation;
  res.rotation        = rotation.Invert();
  res.posttranslation = -posttranslation;

  return res;
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
    newpos.polar  = true;
    newpos.pos.az = newpos.pos.el = 0.0;
    newpos.pos.d  = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
        
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

std::ostream& operator<<(std::ostream& out, const Position& p)
{
  out << "Position(" << p.pos.x << ", " << p.pos.y << ", " << p.pos.z << ", polar = " << p.polar << ")";
  return out;
}

/*--------------------------------------------------------------------------------*/
/** Return angle between two vectors
 */
/*--------------------------------------------------------------------------------*/
double Angle(const Position& obj1, const Position& obj2)
{
  Position pos1 = obj1.Unit();
  Position pos2 = obj2.Unit();
  double   dot  = DotProduct(pos1, pos2);
  if ((dot < -1.01) || (dot > 1.01))
  {
    BBCERROR("Dot product of (%0.3lf, %0.3lf, %0.3lf) and (%0.3lf, %0.3lf, %0.3lf) = %0.6lf",
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

bool Evaluate(const std::string& str, Position& val)
{
  Position pos;
  bool success = false;

  if (sscanf(str.c_str(), "polar (%lf,%lf) x%lfm", &pos.pos.az, &pos.pos.el, &pos.pos.d) == 3)
  {
    pos.polar = true;
    success = true;
  }
  else if (sscanf(str.c_str(), "cart (%lfm,%lfm,%lfm)", &pos.pos.x, &pos.pos.y, &pos.pos.z) == 3)
  {
    pos.polar = false;
    success = true;
  }

  if (success) val = pos;

  return success;
}


bool Evaluate(const std::string& str, Quaternion& val)
{
  Quaternion rot;
  bool success = false;

  if (sscanf(str.c_str(), "%lf,%lf,%lf,%lf", &rot.w, &rot.x, &rot.y, &rot.z) == 4)
  {
    val = rot;
    success = true;
  }

  return success;
}

std::string StringFrom(const Position& val)
{
  return val.ToString();
}

std::string StringFrom(const Quaternion& val)
{
  return val.ToString();
}

#if ENABLE_JSON
bool FromJSON(const json_spirit::mValue& _val, Position& val)
{
  bool success = (_val.type() == json_spirit::obj_type);

  if (success)
  {
    json_spirit::mObject::iterator it;
    json_spirit::mObject obj = _val.get_obj();
    Position pos;

    if ((it = obj.find("polar")) != obj.end())
    {
      FromJSON(it->second, pos.polar);
    }

    if (pos.polar)
    {
      if (success && ((it = obj.find("az")) != obj.end()))
      {
        success = FromJSON(it->second, pos.pos.az);
      }
      if (success && ((it = obj.find("el")) != obj.end()))
      {
        success = FromJSON(it->second, pos.pos.el);
      }
      if (success && ((it = obj.find("d")) != obj.end()))
      {
        success = FromJSON(it->second, pos.pos.d);
      }
    }
    else
    {
      if (success && ((it = obj.find("x")) != obj.end()))
      {
        success = FromJSON(it->second, pos.pos.x);
      }
      if (success && ((it = obj.find("y")) != obj.end()))
      {
        success = FromJSON(it->second, pos.pos.y);
      }
      if (success && ((it = obj.find("z")) != obj.end()))
      {
        success = FromJSON(it->second, pos.pos.z);
      }
    }

    if (success) val = pos;
  }

  return success;
}

json_spirit::mValue ToJSON(const Position& val)
{
  json_spirit::mObject obj;

  obj["polar"] = val.polar;
  if (val.polar)
  {
    obj["az"] = val.pos.az;
    obj["el"] = val.pos.el;
    obj["d"]  = val.pos.d;
  }
  else
  {
    obj["x"] = val.pos.x;
    obj["y"] = val.pos.y;
    obj["z"] = val.pos.z;
  }

  return obj;
}

bool FromJSON(const json_spirit::mValue& _val, Quaternion& val)
{
  bool success = (_val.type() == json_spirit::obj_type);

  if (success)
  {
    json_spirit::mObject::iterator it;
    json_spirit::mObject obj = _val.get_obj();
    Quaternion rot;

    if (success && ((it = obj.find("w")) != obj.end()))
    {
      success = FromJSON(it->second, rot.w);
    }
    if (success && ((it = obj.find("x")) != obj.end()))
    {
      success = FromJSON(it->second, rot.x);
    }
    if (success && ((it = obj.find("y")) != obj.end()))
    {
      success = FromJSON(it->second, rot.y);
    }
    if (success && ((it = obj.find("z")) != obj.end()))
    {
      success = FromJSON(it->second, rot.z);
    }

    if (success) val = rot;
  }

  return success;
}

json_spirit::mValue ToJSON(const Quaternion& val)
{
  json_spirit::mObject obj;

  obj["w"] = val.w;
  obj["x"] = val.x;
  obj["y"] = val.y;
  obj["z"] = val.z;

  return obj;
}

#endif

BBC_AUDIOTOOLBOX_END
