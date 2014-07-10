#ifndef __PERFORMANCE_MONITOR__
#define __PERFORMANCE_MONITOR__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Simple averaging performance monitor
 *
 * Call 'Start()' at the start of the activity to be measured and 'Stop()' at the end
 *
 * GetSecondsTaken() will give the averaged time taken for the activity
 * GetSecondsElapsed() will give the averaged time between the start of each activity
 *
 * GetUsage() gives taken / elapsed which gives usage
 *
 * When NeedsReport() returns true, the results can be reported (and then Reported() called)
 * The rate of reporting is dictated from the 'report' parameter in the constructor and defaults to 1s
 */
/*--------------------------------------------------------------------------------*/
class PerformanceMonitor {
public:
  PerformanceMonitor(uint_t avglen = 10, uint32_t report = 1000);
  ~PerformanceMonitor();

  /*--------------------------------------------------------------------------------*/
  /** Reset the monitor
   */
  /*--------------------------------------------------------------------------------*/
  void Reset();

  /*--------------------------------------------------------------------------------*/
  /** Start performance measurement
   */
  /*--------------------------------------------------------------------------------*/
  void Start();

  /*--------------------------------------------------------------------------------*/
  /** Stop performance measurement
   */
  /*--------------------------------------------------------------------------------*/
  void Stop();

  /*--------------------------------------------------------------------------------*/
  /** Return the averaged time taken for the activity
   */
  /*--------------------------------------------------------------------------------*/
  double GetSecondsTaken()   const {return taken_sec;}

  /*--------------------------------------------------------------------------------*/
  /** Return the averaged time between the start of each activity
   */
  /*--------------------------------------------------------------------------------*/
  double GetSecondsElapsed() const {return elapsed_sec;}

  /*--------------------------------------------------------------------------------*/
  /** Return averaged taken / elapsed
   */
  /*--------------------------------------------------------------------------------*/
  double GetUsage() const {return GetSecondsTaken() / GetSecondsElapsed();}

  /*--------------------------------------------------------------------------------*/
  /** Return averaged taken / elapsed as an integer percent
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetUsagePercent() const {return usage;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether a report of results should be made
   */
  /*--------------------------------------------------------------------------------*/
  bool NeedsReport() const {return ((GetTickCount() - reporttick) >= reportinterval);}

  /*--------------------------------------------------------------------------------*/
  /** Reset the report time
   */
  /*--------------------------------------------------------------------------------*/
  void Reported() {reporttick = GetTickCount();}

protected:
  typedef uint64_t perftime_t;

  perftime_t GetCurrent() const;

  typedef struct {
    perftime_t start;
    perftime_t stop;
    perftime_t taken;
    uint_t     usage;
  } TIMING;

protected:
  TIMING     *timings;
  uint_t     ntimings, index;
  bool       wrapped;

  perftime_t elapsed;
  perftime_t taken;
  uint_t     usage;

  double     taken_sec;
  double     elapsed_sec;

  uint32_t   reporttick;
  uint32_t   reportinterval;
};

/*--------------------------------------------------------------------------------*/
/** Simple class wrapper for performance start/stop
 *
 */
/*--------------------------------------------------------------------------------*/
class MonitorPerformance {
public:
  MonitorPerformance(PerformanceMonitor& _perfmon) : perfmon(_perfmon) {perfmon.Start();}
  ~MonitorPerformance() {perfmon.Stop();}

protected:
  PerformanceMonitor& perfmon;
};

BBC_AUDIOTOOLBOX_END

#endif
