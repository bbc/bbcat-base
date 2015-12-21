#ifndef __3D_POSITION__
#define __3D_POSITION__

#include <math.h>
#include <string>

#if ENABLE_JSON
#include <json_spirit/json_spirit.h>
#endif

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
  
  /*--------------------------------------------------------------------------------*/
  /** Print a description of this object
   */
  /*--------------------------------------------------------------------------------*/
  friend std::ostream& operator<<(std::ostream& out, const Position& p);

  /*--------------------------------------------------------------------------------*/
  /** Extract position (either polar or cartesian) from a set of parameters
   */
  /*--------------------------------------------------------------------------------*/
  bool GetFromParameters(const ParameterSet& parameters, const std::string& name);

  /*--------------------------------------------------------------------------------*/
  /** Set position (either polar or cartesian) in a set of parameters
   *
   * @note if radians = true, polar values are set in radians
   */
  /*--------------------------------------------------------------------------------*/
  void SetParameters(ParameterSet& parameters, const std::string& name, bool radians = false) const;

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
    double elements[3];   // {az,el,d} or {x,y,z}
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
  /** Construct from raw coeffs (like SetFromCoeffs)
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion(double _w = 1.0, double _x = 0.0, double _y = 0.0, double _z = 0.0);
  /*--------------------------------------------------------------------------------*/
  /** Construct from angle/axis (like SetFromAngleAxis)
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion(double phi, const Position& vec);
  /*--------------------------------------------------------------------------------*/
  /** Construct Quaternion representation of 3D position
   */
  /*--------------------------------------------------------------------------------*/
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
  Quaternion& operator = (const Position& vec);

  /*--------------------------------------------------------------------------------*/
  /** Comparison operators
   */
  /*--------------------------------------------------------------------------------*/
  friend bool operator == (const Quaternion& obj1, const Quaternion& obj2);
  friend bool operator != (const Quaternion& obj1, const Quaternion& obj2) {return !operator == (obj1, obj2);}

  /*--------------------------------------------------------------------------------*/
  /** Set the Quaternion from raw coeffs
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion& SetFromCoeffs(double _w, double _x, double _y, double _z);

  /*--------------------------------------------------------------------------------*/
  /** Set the Quaternion from an axis and an angle (in degrees).
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion& SetFromAngleAxis(double angle, double _x, double _y, double _z);
  Quaternion& SetFromAngleAxis(double angle, const Position& axis);

  /*--------------------------------------------------------------------------------*/
  /** Multiply operator - scalar multiply
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator * (const Quaternion& obj1, double val);
  Quaternion& operator *= (double val) {*this = *this * val; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Multiply operator - apply second rotation to first
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator * (const Quaternion& obj1, const Quaternion& obj2);
  Quaternion& operator *= (const Quaternion& obj) {*this = *this * obj; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Divide operator - scalar divide
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator / (const Quaternion& obj1, double val);
  Quaternion& operator /= (double val) {*this = *this / val; return *this;}

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

  /*--------------------------------------------------------------------------------*/
  /** Add a Quaternion to this one
   */
  /*--------------------------------------------------------------------------------*/
  void operator += (const Quaternion &quaternion);

  /*--------------------------------------------------------------------------------*/
  /** Add two Quaternions
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator + (const Quaternion &q0, const Quaternion &q1);

  /*--------------------------------------------------------------------------------*/
  /** Subtract a Quaternion from this one
   */
  /*--------------------------------------------------------------------------------*/
  void operator -= (const Quaternion &quaternion);

  /*--------------------------------------------------------------------------------*/
  /** Subtract two Quaternions (q0-q1)
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion operator - (const Quaternion &q0, const Quaternion &q1);

  /*--------------------------------------------------------------------------------*/
  /** Negate the Quaternion
   *
   * @note in rotation terms, the Quaternion still does the same but rotates in the opposite direction
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion operator - () const;

  /*--------------------------------------------------------------------------------*/
  /** Invert the rotation represented by the Quaternion
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion Invert() const;

  /*--------------------------------------------------------------------------------*/
  /** Return the scalar (dot) product as generated for normal 4D vectors
   */
  /*--------------------------------------------------------------------------------*/
  double ScalarProduct(const Quaternion &q) const;

  /*--------------------------------------------------------------------------------*/
  /** Determine the logarithm of this quaternion (only valid for unit quaternions) : axis*angle
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion Log() const;

  /*--------------------------------------------------------------------------------*/
  /** Determine the outcome of using this quaternion as an exponent e^q - note: due to noncommutativity of quaternion multiplication,
   * standard identities for exponential and logarthmic functions do not apply.
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion Exp() const;

  /*--------------------------------------------------------------------------------*/
  /** Return a normalised version of the quaternion. The ensures that it represents a
   * rotation, can be used to check rounding errors.
   */
  /*--------------------------------------------------------------------------------*/
  Quaternion Normalised() const;

  /*--------------------------------------------------------------------------------*/
  /** Perform Linear intERPolation between two unit quaternions.
   *
   * @note Assumes that quaternions have unit-length i.e. have been normalised.
   * This is to ensure that this method is fast.
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion Lerp(const Quaternion& q0, const Quaternion& q1, double t);

  /*--------------------------------------------------------------------------------*/
  /** Perform Spherical Linear intERPolation between two unit quaternions.
   *
   * @note Assumes that quaternions have unit-length i.e. have been normalised.
   * This is to ensure that this method is fast.
   */
  /*--------------------------------------------------------------------------------*/
  friend Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, double t);

  /*--------------------------------------------------------------------------------*/
  /** Extract rotation (either angular or pure Quaternion) from a set of parameters
   */
  /*--------------------------------------------------------------------------------*/
  bool GetFromParameters(const ParameterSet& parameters, const std::string& name);

  /*--------------------------------------------------------------------------------*/
  /** Set rotation (either angular or pure Quaternion) in a set of parameters
   */
  /*--------------------------------------------------------------------------------*/
  void SetParameters(ParameterSet& parameters, const std::string& name) const;

//  void ToRotationMatrix(std::vector<double>& r) const
//  {
//    r.resize(9);
//    r[0] = 1 - (2 * y * y + 2 * z * z);
//    r[1] = 2 * x * y + 2 * z * w;
//    r[2] = 2 * x * z - 2 * y * w;
//    r[3] = 2 * x * y - 2 * z * w;
//    r[4] = 1 - (2 * x * x + 2 * z * z);
//    r[5] = 2 * y * z + 2 * x * w;
//    r[6] = 2 * x * z + 2 * y * w;
//    r[7] = 2 * y * z - 2 * x * w;
//    r[8] = 1 - (2 * x * x + 2 * y * y);
//  }

  /*--------------------------------------------------------------------------------*/
  /** Generate friendly text string
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToString() const;

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
  /** Comparison operators
   */
  /*--------------------------------------------------------------------------------*/
  friend bool operator == (const PositionTransform& obj1, const PositionTransform& obj2);
  friend bool operator != (const PositionTransform& obj1, const PositionTransform& obj2) {return !operator == (obj1, obj2);}

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

  /*--------------------------------------------------------------------------------*/
  /** Negation operator
   */
  /*--------------------------------------------------------------------------------*/
  PositionTransform operator - () const;

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

extern bool        Evaluate(const std::string& str, Position& val);
extern bool        Evaluate(const std::string& str, Quaternion& val);
extern std::string StringFrom(const Position& val);
extern std::string StringFrom(const Quaternion& val);

#if ENABLE_JSON
extern bool                FromJSON(const json_spirit::mValue& _val, Position& val);
extern json_spirit::mValue ToJSON(const Position& val);

extern bool                FromJSON(const json_spirit::mValue& _val, Quaternion& val);
extern json_spirit::mValue ToJSON(const Quaternion& val);
#endif

BBC_AUDIOTOOLBOX_END

#endif
