#ifndef __3D_POSITION__
#define __3D_POSITION__

#include <math.h>
#include <string>

#include "misc.h"
#include "ParameterSet.h"

BBC_AUDIOTOOLBOX_START

class PositionTransform;

/*--------------------------------------------------------------------------------*/
/** Position object - holds polar or cartesian co-ordinates with all angles in degrees
 *
 * Allows conversion between polar and cartesian as well as arithmetic operations
 *
 */
/*--------------------------------------------------------------------------------*/
class Position {
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
  Position(double x = 0.0, double y = 0.0, double z = 0.0) : polar(false) {
    memset(&pos, 0, sizeof(pos));
    pos.x = x; pos.y = y; pos.z = z;
  }
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
  virtual Position Polar() const;

  /*--------------------------------------------------------------------------------*/
  /** Return the same position but as cartesian co-ordinates
   */
  /*--------------------------------------------------------------------------------*/
  virtual Position Cart() const;

  /*--------------------------------------------------------------------------------*/
  /** Limit azimuth and elevation
   */
  /*--------------------------------------------------------------------------------*/
  virtual void LimitAngles();

  /*--------------------------------------------------------------------------------*/
  /** Assignment
   */
  /*--------------------------------------------------------------------------------*/
  virtual Position& operator = (const Position& obj) {polar = obj.polar; pos = obj.pos; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Translate the current position by the supplied position in cartesian space
   *
   * @note the object remains in the same co-ordinate system as it was
   */
  /*--------------------------------------------------------------------------------*/
  virtual Position& operator += (const Position& obj);
  virtual Position& operator -= (const Position& obj);

  /*--------------------------------------------------------------------------------*/
  /** Scale/reduce the current position
   */
  /*--------------------------------------------------------------------------------*/
  virtual Position& operator *= (double val);
  virtual Position& operator /= (double val) {return operator *= (1.0 / val);}
  virtual Position& operator *= (const double vals[3]);
  virtual Position& operator *= (const double vals[3][3]);

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
  virtual Position operator - () const;

  /*--------------------------------------------------------------------------------*/
  /** Apply position transform
   */
  /*--------------------------------------------------------------------------------*/
  virtual Position& operator *= (const PositionTransform& trans);
    
  /*--------------------------------------------------------------------------------*/
  /** Apply position transform
   */
  /*--------------------------------------------------------------------------------*/
  friend Position operator * (const Position& pos, const PositionTransform& trans);

  /*--------------------------------------------------------------------------------*/
  /** Generate friendly text string
   */
  /*--------------------------------------------------------------------------------*/
  virtual std::string ToString() const;

  /*--------------------------------------------------------------------------------*/
  /** Return unit vector version of this Position
   */
  /*--------------------------------------------------------------------------------*/
  virtual Position Unit() const;

  /*--------------------------------------------------------------------------------*/
  /** Return modulus (distance) of Position from origin
   */
  /*--------------------------------------------------------------------------------*/
  virtual double Mod() const {return polar ? pos.d : sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);}

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

  bool polar;               // true if co-ordinates are polar
  union {
    struct {
      double az, el, d;     // azimuth (degrees), elevation (degrees) and distance (m)
    };
    struct {
      double x, y, z;       // co-ordinates in m
    };
  } pos;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Position transformation class
 *
 * Transform consists of a translation, rotation around each axis and then another translation
 *
 */
/*--------------------------------------------------------------------------------*/

class PositionTransform {
public:
  PositionTransform();
  PositionTransform(const PositionTransform& obj);
  virtual ~PositionTransform() {}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  PositionTransform& operator = (const PositionTransform& obj);

  /*--------------------------------------------------------------------------------*/
  /** Add transform to this transform
   */
  /*--------------------------------------------------------------------------------*/
  PositionTransform& operator += (const PositionTransform& obj);
  friend PositionTransform operator + (const PositionTransform& obj1, const PositionTransform& obj2) {
    PositionTransform res = obj1;
    res += obj2;
    return res;
  }

  /*--------------------------------------------------------------------------------*/
  /** Subtract transform to this transform
   */
  /*--------------------------------------------------------------------------------*/
  PositionTransform& operator -= (const PositionTransform& obj);
  friend PositionTransform operator - (const PositionTransform& obj1, const PositionTransform& obj2) {
    PositionTransform res = obj1;
    res -= obj2;
    return res;
  }

  Position pretranslation;
  double   xrotation; // rotation around the x-axis
  double   yrotation; // rotation around the y-ayis
  double   zrotation; // rotation around the z-azis
  Position posttranslation;

  /*--------------------------------------------------------------------------------*/
  /** Apply transform to position
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Transform(Position& pos) const;

protected:
  /*--------------------------------------------------------------------------------*/
  /** Rotate x and y by angle
   */
  /*--------------------------------------------------------------------------------*/
  void Rotate(double& x, double& y, double angle) const;
};

BBC_AUDIOTOOLBOX_END

#endif
