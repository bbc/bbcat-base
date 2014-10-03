#ifndef __3D_POSITION__
#define __3D_POSITION__

#include <math.h>
#include <string>

#include "misc.h"
#include "ParameterSet.h"

BBC_AUDIOTOOLBOX_START

class PositionTransform;
class ScreenTransform;
class Quaternion;

/*--------------------------------------------------------------------------------*/
/** Position object - holds polar or cartesian co-ordinates with all angles in degrees
 *
 * Allows conversion between polar and cartesian as well as arithmetic operations
 *
 */
/*--------------------------------------------------------------------------------*/
class Position
{
public:
  // Polar:
  // For azimuth:   0 == straight ahead, +ve az travels anti-clockwise when viewed from above
  // For elevation: 0 == level with centre, +ve el travels up

  // Cartesian:
  // For x: 1 == right
  // For y: 1 == forward
  // For z: 1 == up

  // Conversions:
  //  az  el    x  y  z
  //   0   0    0  1  0
  //   0  90    0  0  1
  //   0 -90    0  0 -1
  //  90   0   -1  0  0
  //  90  90    0  0  1
  //  90 -90    0  0 -1
  // -90   0    1  0  0
  // -90  90    0  0  1
  // -90 -90    0  0 -1

  // x = -sin(az) * cos(el)
  // y =  cos(az) * cos(el)
  // z =  sin(el)

  // el = asin(z)
  // az = atan(-x / y)

  /*--------------------------------------------------------------------------------*/
  /** Default constructor, defaults to origin and cartesian co-ordinates
   */
  /*--------------------------------------------------------------------------------*/
  Position(double x = 0.0, double y = 0.0, double z = 0.0);
  /*--------------------------------------------------------------------------------*/
  /** Copy constructor
   */
  /*--------------------------------------------------------------------------------*/
  Position(const Position& obj) : polar(obj.polar),
                                  pos(obj.pos) {}
    
  /*--------------------------------------------------------------------------------*/
  /** Return the same position but as polar co-ordinates
   */
  /*--------------------------------------------------------------------------------*/
  Position Polar() const;

  /*--------------------------------------------------------------------------------*/
  /** Return the same position but as cartesian co-ordinates
   */
  /*--------------------------------------------------------------------------------*/
  Position Cart() const;

  /*--------------------------------------------------------------------------------*/
  /** Limit azimuth and elevation
   */
  /*--------------------------------------------------------------------------------*/
  void LimitAngles();

  /*--------------------------------------------------------------------------------*/
  /** Assignment
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator = (const Position& obj) {polar = obj.polar; pos = obj.pos; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Translate the current position by the supplied position in cartesian space
   *
   * @note the object remains in the same co-ordinate system as it was
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator += (const Position& obj);
  Position& operator -= (const Position& obj);

  /*--------------------------------------------------------------------------------*/
  /** Scale/reduce the current position
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator *= (double val);
  Position& operator /= (double val) {return operator *= (1.0 / val);}
  Position& operator *= (const double vals[3]);
  Position& operator *= (const double vals[3][3]);

  /*--------------------------------------------------------------------------------*/
  /** Binary arithmetic operations, doesn't modify supplied objects
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator + (const Position& obj1, const Position& obj2)    {Position res = obj1; res += obj2; return res;}
  friend Position operator - (const Position& obj1, const Position& obj2)    {Position res = obj1; res -= obj2; return res;}
  friend Position operator * (const Position& obj1, double val)              {Position res = obj1; res *= val;  return res;}
  friend Position operator / (const Position& obj1, double val)              {Position res = obj1; res /= val;  return res;}
  friend Position operator * (const Position& obj1, const double vals[3])    {Position res = obj1; res *= vals; return res;}
  friend Position operator * (const Position& obj1, const double vals[3][3]) {Position res = obj1; res *= vals; return res;}

  /*--------------------------------------------------------------------------------*/
  /** Comparison operators
   */
  /*--------------------------------------------------------------------------------*/
  friend bool operator == (const Position& obj1, const Position& obj2);
  friend bool operator != (const Position& obj1, const Position& obj2) {return !operator == (obj1, obj2);}

  /*--------------------------------------------------------------------------------*/
  /** Return antipodean version of position
   */
  /*--------------------------------------------------------------------------------*/
  Position operator - () const;

  /*--------------------------------------------------------------------------------*/
  /** Apply position transform
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator *= (const PositionTransform& trans);
    
  /*--------------------------------------------------------------------------------*/
  /** Apply position transform
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator * (const Position& pos, const PositionTransform& trans);

  /*--------------------------------------------------------------------------------*/
  /** Remove position transform
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator /= (const PositionTransform& trans);
    
  /*--------------------------------------------------------------------------------*/
  /** Remove position transform
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator / (const Position& pos, const PositionTransform& trans);

  /*--------------------------------------------------------------------------------*/
  /** Apply screen transform
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator *= (const ScreenTransform& trans);
    
  /*--------------------------------------------------------------------------------*/
  /** Apply screen transform
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator * (const Position& pos, const ScreenTransform& trans);

  /*--------------------------------------------------------------------------------*/
  /** Remove screen transform
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator /= (const ScreenTransform& trans);
    
  /*--------------------------------------------------------------------------------*/
  /** Remove screen transform
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator / (const Position& pos, const ScreenTransform& trans);

  /*--------------------------------------------------------------------------------*/
  /** Apply rotation
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator *= (const Quaternion& rotation);

  /*--------------------------------------------------------------------------------*/
  /** Remove rotation
   */
  /*--------------------------------------------------------------------------------*/
  Position& operator /= (const Quaternion& rotation);

  /*--------------------------------------------------------------------------------*/
  /** Generate friendly text string
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToString() const;

  /*--------------------------------------------------------------------------------*/
  /** Return unit vector version of this Position
   */
  /*--------------------------------------------------------------------------------*/
  Position Unit() const;

  /*--------------------------------------------------------------------------------*/
  /** Return modulus (distance) of Position from origin
   */
  /*--------------------------------------------------------------------------------*/
  double Mod() const {return polar ? pos.d : sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);}

  /*--------------------------------------------------------------------------------*/
  /** Return dot product of two positions
   */
  /*--------------------------------------------------------------------------------*/
  friend double DotProduct(const Position& obj1, const Position& obj2);
  friend double DotProduct(const Position& obj1, const double obj2[3]);

  /*--------------------------------------------------------------------------------*/
  /** Return angle between two vectors
   */
  /*--------------------------------------------------------------------------------*/
  friend double Angle(const Position& obj1, const Position& obj2);
  friend double AbsAngle(const Position& obj1, const Position& obj2) {return fabs(Angle(obj1, obj2));}

  /*--------------------------------------------------------------------------------*/
  /** Return cross product of two positions
   */
  /*--------------------------------------------------------------------------------*/
  friend Position CrossProduct(const Position& obj1, const Position& obj2);

  bool polar;                 // true if co-ordinates are polar
  union
  {
    struct
    {
      double az, el, d;   // azimuth (degrees), elevation (degrees) and distance (m)
    };
    struct
    {
      double x, y, z;     // co-ordinates in m
    };
  } pos;
};

extern const Position XAxis;
extern const Position YAxis;
extern const Position ZAxis;

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Quaternion class
 *
 */
/*--------------------------------------------------------------------------------*/
class Quaternion
{
public:
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
  Quaternion(double phi = 0.0, double _x = 1.0, double _y = 0.0, double _z = 0.0);
  Quaternion(double phi, const Position& vec);
  Quaternion(const Position& vec);
  /*--------------------------------------------------------------------------------*/
  /** Copy constructor
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion(const Quaternion& obj) : w(obj.w),
                                      x(obj.x),
                                      y(obj.y),
                                      z(obj.z) {}
  ~Quaternion() {}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion& operator = (const Quaternion& obj);
  Quaternion& operator = (const Position& pos);

  /*--------------------------------------------------------------------------------*/
  /** Negate operator - invert rotation
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator - (const Quaternion& obj) {
    Quaternion res;
    res.w =  obj.w;
    res.x = -obj.x;
    res.y = -obj.y;
    res.z = -obj.z;
    return res; 
  }

  /*--------------------------------------------------------------------------------*/
  /** Explicit set functions
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion& Set(double phi, double _x, double _y, double _z);
  Quaternion& Set(double phi, const Position& vec);

  /*--------------------------------------------------------------------------------*/
  /** Multiply operator - apply second rotation to first
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator * (const Quaternion& obj1, const Quaternion& obj2);
  Quaternion& operator *= (const Quaternion& obj) {*this = *this * obj; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Divide operator - remove second rotation from first
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator / (const Quaternion& obj1, const Quaternion& obj2);
  Quaternion& operator /= (const Quaternion& obj) {*this = *this / obj; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Return angle
   *
   * @note returns a value in the range 0 - 180 degrees
   */
  /*--------------------------------------------------------------------------------*/
  double GetAngle() const {return 360.0 / M_PI * acos(w);}

  /*--------------------------------------------------------------------------------*/
  /** Return axis of Quaternion
   */
  /*--------------------------------------------------------------------------------*/
  Position GetAxis() const {return Position(x, y, z);}
  
  /*--------------------------------------------------------------------------------*/
  /** Rotate position by Quaternion
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator * (const Position& pos, const Quaternion& rotation);

  /*--------------------------------------------------------------------------------*/
  /** Reverse rotate position by Quaternion
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator / (const Position& pos, const Quaternion& rotation);

  double w, x, y, z;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Position transformation class
 *
 * Transform consists of a translation, rotation around each axis and then another translation
 *
 */
/*--------------------------------------------------------------------------------*/
class PositionTransform
{
public:
  PositionTransform();
  PositionTransform(const PositionTransform& obj);
  PositionTransform(const Quaternion&        obj);
  ~PositionTransform() {}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  PositionTransform& operator = (const PositionTransform& obj);
  PositionTransform& operator = (const Quaternion&        obj);

  /*--------------------------------------------------------------------------------*/
  /** Add transform to this transform
   */
  /*--------------------------------------------------------------------------------*/
  PositionTransform& operator += (const PositionTransform& obj);
  PositionTransform& operator *= (const Quaternion&        obj) {rotation *= obj; return *this;}
  friend PositionTransform operator + (const PositionTransform& obj1, const PositionTransform& obj2)
  {
    PositionTransform res = obj1;
    res += obj2;
    return res;
  }

  /*--------------------------------------------------------------------------------*/
  /** Subtract transform to this transform
   */
  /*--------------------------------------------------------------------------------*/
  PositionTransform& operator -= (const PositionTransform& obj);
  PositionTransform& operator /= (const Quaternion&        obj) {rotation /= obj; return *this;}
  friend PositionTransform operator - (const PositionTransform& obj1, const PositionTransform& obj2)
  {
    PositionTransform res = obj1;
    res -= obj2;
    return res;
  }

  Position   pretranslation;
  Quaternion rotation;
  Position   posttranslation;

  /*--------------------------------------------------------------------------------*/
  /** Apply transform to position
   */
  /*--------------------------------------------------------------------------------*/
  void ApplyTransform(Position& pos) const;

  /*--------------------------------------------------------------------------------*/
  /** Remove transform to position
   */
  /*--------------------------------------------------------------------------------*/
  void RemoveTransform(Position& pos) const;
};

/*--------------------------------------------------------------------------------*/
/** Screen transformation class
 *
 * Transform consists of scaling and positioning on screen and applying perspective
 *
 */
/*--------------------------------------------------------------------------------*/
class ScreenTransform {
public:
  ScreenTransform();
  ScreenTransform(const ScreenTransform& obj);
  ~ScreenTransform() {}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  ScreenTransform& operator = (const ScreenTransform& obj);

  /*--------------------------------------------------------------------------------*/
  /** Return scale due to perspective for the specified Z co-ordinate
   *
   * @note assumes +ve is coming OUT of the screen towards the viewer
   */
  /*--------------------------------------------------------------------------------*/
  double GetDistanceScale(double z) const {return (z != dist) ? dist / (dist - z) : 1.0;}

  /*--------------------------------------------------------------------------------*/
  /** Return scale due to perspective for the specified position
   */
  /*--------------------------------------------------------------------------------*/
  double GetDistanceScale(const Position& pos) const {return GetDistanceScale(pos.Cart().pos.z);}

  /*--------------------------------------------------------------------------------*/
  /** Apply transform to position
   */
  /*--------------------------------------------------------------------------------*/
  void ApplyTransform(Position& pos) const;

  /*--------------------------------------------------------------------------------*/
  /** Remove transform to position
   */
  /*--------------------------------------------------------------------------------*/
  void RemoveTransform(Position& pos) const;
  
  double cx, cy;        // screen centre
  double sx, sy;        // screen scale
  double dist;          // perspective distance
};

BBC_AUDIOTOOLBOX_END

#endif
