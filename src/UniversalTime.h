#ifndef __UNIVERSAL_TIME__
#define __UNIVERSAL_TIME__

#include <vector>
#include <algorithm>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Interface class for notifcation of timebase updates
 */
/*--------------------------------------------------------------------------------*/
class UniversalTime;
class UniversalTimeUpdateReceiver
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Timebase had changed
   */
  /*--------------------------------------------------------------------------------*/
  virtual void TimebaseUpdated(const UniversalTime *timebase) = 0;
};

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
  UniversalTime(uint64_t den = 1) : time_current(0),
                                    time_offset(0),
                                    time_numerator(0),
                                    time_denominator(den) {}
  UniversalTime(const UniversalTime& obj) : time_current(obj.time_current),
                                            time_offset(obj.time_offset),
                                            time_numerator(obj.time_numerator),
                                            time_denominator(obj.time_denominator) {}
  virtual ~UniversalTime() {}

  /*--------------------------------------------------------------------------------*/
  /** Add update receiver
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddUpdateReceiver(UniversalTimeUpdateReceiver *receiver)
  {
    if (std::find(updatelist.begin(), updatelist.end(), receiver) == updatelist.end()) updatelist.push_back(receiver);
  }

  /*--------------------------------------------------------------------------------*/
  /** Remove update receiver
   */
  /*--------------------------------------------------------------------------------*/
  virtual void RemoveUpdateReceiver(UniversalTimeUpdateReceiver *receiver)
  {
    std::vector<UniversalTimeUpdateReceiver *>::iterator it;
    if ((it = std::find(updatelist.begin(), updatelist.end(), receiver)) != updatelist.end()) updatelist.erase(it);
  }

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  UniversalTime& operator = (const UniversalTime& obj)
  {
    time_current     = obj.time_current;
    time_offset      = obj.time_offset;
    time_numerator   = obj.time_numerator;
    time_denominator = obj.time_denominator;
    return *this;
  }

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
  /** Reset time to zero
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Reset() {time_offset = 0; time_numerator = 0;}

  /*--------------------------------------------------------------------------------*/
  /** Set numerator
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Set(uint64_t num)                                {time_numerator = num; UpdateTime();}

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
  /** Set numerator
   */
  /*--------------------------------------------------------------------------------*/
  virtual UniversalTime& operator = (uint64_t num)              {time_numerator  = num; UpdateTime(); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Add to numerator
   */
  /*--------------------------------------------------------------------------------*/
  virtual UniversalTime& operator += (uint64_t inc)             {time_numerator += inc; UpdateTime(); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Add one object to another using offset
   */
  /*--------------------------------------------------------------------------------*/
  virtual UniversalTime& operator += (const UniversalTime& obj) {time_offset    += obj.GetTime(); UpdateTime(); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Return calculated time in raw values 
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GetRawTime() const {return time_numerator;}
  
  /*--------------------------------------------------------------------------------*/
  /** Return calculated time in ns
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GetTime()    const {return time_current;}
  operator uint64_t()   const {return time_current;}

  /*--------------------------------------------------------------------------------*/
  /** Return calculated time in s
   */
  /*--------------------------------------------------------------------------------*/
  double GetTimeSeconds() const {return 1.0e-9 * (double)time_current;}

  /*--------------------------------------------------------------------------------*/
  /** Perform explicit calculation using denominator
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t Calc(uint64_t num)        const {return (uint64_t)((1000000000ULL * (ullong_t)num) / time_denominator);}
  double   CalcSeconds(uint64_t num) const {return 1.0e-9 * (double)Calc(num);}

  friend uint64_t operator * (const UniversalTime& timebase, uint64_t num)  {return timebase.Calc(num);}
  friend uint64_t operator * (uint64_t num, const UniversalTime& timebase)  {return timebase.Calc(num);}

  /*--------------------------------------------------------------------------------*/
  /** Perform inverse conversion from time in ns
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t Invert(uint64_t val)      const {return (uint64_t)(((ullong_t)val * (ullong_t)time_denominator) / 1000000000ULL);}

  friend uint64_t operator / (uint64_t time, const UniversalTime& timebase) {return timebase.Invert(time);}

protected:
  void UpdateTime()
  {
    uint_t i;
    time_current = time_offset + (uint64_t)((1000000000ULL * (ullong_t)time_numerator) / time_denominator);
    for (i = 0; i < updatelist.size(); i++) updatelist[i]->TimebaseUpdated(this);
  }

protected:
  uint64_t time_current;
  uint64_t time_offset;
  uint64_t time_numerator;
  uint64_t time_denominator;
  std::vector<UniversalTimeUpdateReceiver *> updatelist;
};

BBC_AUDIOTOOLBOX_END

#endif
