#ifndef __INTERPOLATOR__
#define __INTERPOLATOR__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Simple interpolator class (can be used as controller for other interpolators)
 */
/*--------------------------------------------------------------------------------*/
class Interpolator
{
public:
  Interpolator(float _target = 0.0, float _current = 0.0) : target(_target),
                                                            current(_current) {}
  Interpolator(const Interpolator& obj) : target(obj.target),
                                          current(obj.current) {}
  ~Interpolator() {}

  /*--------------------------------------------------------------------------------*/
  /** Return whether this object is or will be non-silent
   */
  /*--------------------------------------------------------------------------------*/
  bool NonZero() const {return ((current != 0.0) || (target != 0.0));}

  /*--------------------------------------------------------------------------------*/
  /** Set current value
   */
  /*--------------------------------------------------------------------------------*/
  Interpolator& SetCurrent(float _current) {current = _current; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Set new target
   */
  /*--------------------------------------------------------------------------------*/
  Interpolator& SetTarget(float _target) {target = _target; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Set new target
   */
  /*--------------------------------------------------------------------------------*/
  Interpolator& operator = (float _target) {target = _target; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  Interpolator& operator = (const Interpolator& obj) {target = obj.target; current = obj.current; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Interpolate current value towards target at the specified rate
   */
  /*--------------------------------------------------------------------------------*/
  Interpolator& operator += (float inc) {current = (target >= current) ? MIN(current + inc, target) : MAX(current - inc, target); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Return current value
   */
  /*--------------------------------------------------------------------------------*/
  operator float () const {return current;}

  /*--------------------------------------------------------------------------------*/
  /** Return target
   */
  /*--------------------------------------------------------------------------------*/
  float GetTarget() const {return target;}

protected:
  float target;
  float current;
};

/*--------------------------------------------------------------------------------*/
/** More complex interpolation class
 *
 * Uses an instance of the above to create more reliable relationship between interpolation values
 *
 * In particular, it assumes the interpolation controller starts at 1.0 and moves towards 0.0 using
 * the controller's value as a scaling factor:
 *
 * current = target - controller_value * diff
 *
 * Where diff is the difference between target and the starting value
 *
 * This method ensures that the interpolation becomes MORE accurate as it approaches the target and
 * is guaranteed to reach the target at the right time.  This is CRITICAL when using interpolation
 * on coeffs for biquads - all 5 coeffs can be controlled by the same controller and so guarateed
 * to not go bang! as with other interpolation methods
 *
 * The double operator function can be overloaded to create non-linear relationships between time and value
 * (although the endpoints of the controller MUST have the same effect but between the endpoints the relationship can be arbitrary) 
 */
/*--------------------------------------------------------------------------------*/
class ComplexInterpolator
{
public:
  ComplexInterpolator(const Interpolator& _controller, double _current = 0.0, double _target = 0.0) : controller(_controller),
                                                                                                      target(_target),
                                                                                                      diff(_target - _current) {}
  ComplexInterpolator(const ComplexInterpolator& obj) : controller(obj.controller),
                                                        target(obj.target),
                                                        diff(obj.diff) {}
  virtual ~ComplexInterpolator() {}

  /*--------------------------------------------------------------------------------*/
  /** Set new target
   *
   * @note it is CRITICAL that the controller is reset to 1 after new targets have been set
   * @note this means ALL controlled interpolators must be updated at the same time
   */
  /*--------------------------------------------------------------------------------*/
  ComplexInterpolator& operator = (double _target) {double current = *this; target = _target; diff = target - current; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Reset difference if target has not changed but the controller is going to be reset
   */
  /*--------------------------------------------------------------------------------*/
  ComplexInterpolator& Reset() {*this = target; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Return current value
   */
  /*--------------------------------------------------------------------------------*/
  virtual operator double() const {return target - controller * diff;}

  /*--------------------------------------------------------------------------------*/
  /** Return target
   */
  /*--------------------------------------------------------------------------------*/
  double GetTarget() const {return target;}

protected:
  const Interpolator& controller;
  double target;
  double diff;
};

BBC_AUDIOTOOLBOX_END

#endif
