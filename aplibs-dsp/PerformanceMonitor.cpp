
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

#define DEBUG_LEVEL 1
#include "PerformanceMonitor.h"

BBC_AUDIOTOOLBOX_START

PerformanceMonitor::PerformanceMonitor(uint_t avglen, uint32_t report) :
  timings(NULL),
  ntimings(avglen),
  index(0),
  wrapped(false),
  elapsed(0),
  taken(0),
  usage(0),
  taken_sec(0.0),
  elapsed_sec(0.0),
  reporttick(0),
  reportinterval(report)
{
  if ((timings = new TIMING[ntimings]) != NULL)
  {
    memset(timings, 0, ntimings * sizeof(*timings));
  }
}

PerformanceMonitor::~PerformanceMonitor()
{
  if (timings) delete[] timings;
}

PerformanceMonitor::perftime_t PerformanceMonitor::GetCurrent() const
{
#ifdef __MACH__
  static mach_timebase_info_data_t timebase;
  static bool inited = false;

  if (!inited) {
    mach_timebase_info(&timebase);
    inited = true;
  }

  uint64_t tick = mach_absolute_time();
  tick = (tick * timebase.numer) / timebase.denom;

  return (perftime_t)tick;
#else
  struct timespec timespec;

#ifdef ANDROID
  clock_gettime(CLOCK_MONOTONIC_HR, &timespec);
#elif defined(__CYGWIN__)
  clock_gettime(CLOCK_MONOTONIC, &timespec);
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
#endif

  return (perftime_t)timespec.tv_sec * (perftime_t)1000000000 + (perftime_t)timespec.tv_nsec;
#endif
}

/*--------------------------------------------------------------------------------*/
/** Reset the monitor
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::Reset()
{
  memset(timings, 0, ntimings * sizeof(*timings));
  index   = 0;
  wrapped = false;
  elapsed = taken = 0;
  usage   = 0;

  taken_sec = elapsed_sec = 0.0;

  reporttick = 0;
}

/*--------------------------------------------------------------------------------*/
/** Start performance measurement
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::Start()
{
  TIMING& timing = timings[index];
  perftime_t t = GetCurrent();

  if (!reporttick) reporttick = GetTickCount();

  // calc elapsed time based on how far through the array of timings we are
  if (wrapped)    elapsed = t - timing.start;
  else if (index) elapsed = t - timings[0].start;

  // update percentage usage if possible
  if (elapsed) timing.usage = usage = (uint_t)((100 * taken) / elapsed);

  // update start time
  timing.start = t;

  double scale = 1.0e-9;
  // average measurements based on how far through the array of timings we are
  if (wrapped)    scale /= (double)ntimings;    // average over entire array
  else if (index) scale /= (double)index;       // average over data so far

  taken_sec   = (double)taken   * scale;
  elapsed_sec = (double)elapsed * scale;
}

/*--------------------------------------------------------------------------------*/
/** Stop performance measurement
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::Stop()
{
  TIMING& timing = timings[index];
  perftime_t t = GetCurrent();

  // remove old taken value from running average
  taken -= timing.taken;
  // calculate new taken value
  timing.stop  = t;
  timing.taken = t - timing.start;
  // add new taken value to running average
  taken += timing.taken;

  // detect wrap-around and buffer as wrapped (for full running averages)
  if ((++index) == ntimings)
  {
    wrapped = true;
    index   = 0;
  }
}

BBC_AUDIOTOOLBOX_END
