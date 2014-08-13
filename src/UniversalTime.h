#ifndef __UNIVERSAL_TIME__
#define __UNIVERSAL_TIME__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A simple class to provide a ns universal time handler
 *
 * It consists of an offset (in ns) plus a numerator/denominator pair representing
 * the additiona time in seconds
 *
 * That is: time(ns) = offset(ns) + 1e9 * numerator / denominator
 *
 * With ns resolution the object can handle more that 500 years worth of time
 *
 * For sample base times, the denominator should be set at the sample rate and
 * then the numerator can simply count samples
 */
/*--------------------------------------------------------------------------------*/
class UniversalTime
{
public:
  UniversalTime() : time_current(0),
                    time_offset(0),
                    time_numerator(0),
                    time_denominator(1) {}
  virtual ~UniversalTime() {}

  /*--------------------------------------------------------------------------------*/
  /** Change denominator -> use offset to save current ns time and reset numerator to 0
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetDenominator(uint64_t den) {
    if (den != time_denominator)
    {
      time_offset      = GetTime();
      time_numerator   = 0;
      time_denominator = den;
      UpdateTime();
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Add to numerator
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Add(uint64_t inc)                                {time_numerator += inc; UpdateTime();}

  /*--------------------------------------------------------------------------------*/
  /** Add nano seconds to offset
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddNanoSeconds(uint64_t ns)                      {time_offset    += ns;  UpdateTime();}

  /*--------------------------------------------------------------------------------*/
  /** Add to numerator
   */
  /*--------------------------------------------------------------------------------*/
  virtual UniversalTime& operator += (uint64_t inc)             {time_numerator += inc; UpdateTime();}

  /*--------------------------------------------------------------------------------*/
  /** Add one object to another using offset
   */
  /*--------------------------------------------------------------------------------*/
  virtual UniversalTime& operator += (const UniversalTime& obj) {time_offset    += obj.GetTime(); UpdateTime();}
  
  /*--------------------------------------------------------------------------------*/
  /** Return calculated time in ns
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GetTime()  const {return time_current;}
  operator uint64_t() const {return time_current;}

protected:
  void UpdateTime() {time_current = time_offset + (uint64_t)((1000000000ULL * (ullong_t)time_numerator) / time_denominator);}

protected:
  uint64_t time_current;
  uint64_t time_offset;
  uint64_t time_numerator;
  uint64_t time_denominator;
};

BBC_AUDIOTOOLBOX_END

#endif
