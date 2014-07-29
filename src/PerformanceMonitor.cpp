
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <string>

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

#define DEBUG_LEVEL 2
#include "PerformanceMonitor.h"

#define DISP(t) ((double)(t) * 1.0e-9)

#define MEASURE_PERFORMANCE_BY_DEFAULT false
#define LOG_PERFORMANCE_BY_DEFAULT     false
#define REPORT_PERFORMANCE_BY_DEFAULT  false

BBC_AUDIOTOOLBOX_START

PerformanceMonitor::PerformanceMonitor(uint_t _avglen) :
  t0(0),
  avglen(_avglen),
  fp(NULL),
  measure(MEASURE_PERFORMANCE_BY_DEFAULT),
  logtofile(LOG_PERFORMANCE_BY_DEFAULT),
  reportatend(REPORT_PERFORMANCE_BY_DEFAULT)
{
}

PerformanceMonitor::~PerformanceMonitor()
{
  std::map<std::string,TIMING_DATA>::iterator it;

  if (fp) fclose(fp);

  for (it = timings.begin(); it != timings.end(); ++it)
  {
    TIMING_DATA& data = it->second;

    if (data.fp) fclose(data.fp);
  }

  if (reportatend)
  {
    std::string res = GetReportEx();

    DEBUG("%s", res.c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Get access to Performance Monitor singleton
 */
/*--------------------------------------------------------------------------------*/
PerformanceMonitor& PerformanceMonitor::Get()
{
  static PerformanceMonitor perfmon;
  return perfmon;
}

/*--------------------------------------------------------------------------------*/
/** Return textual performance report
 */
/*--------------------------------------------------------------------------------*/
std::string PerformanceMonitor::GetReportEx()
{
  ThreadLock lock(tlock);
  std::string res;

  if (timings.end() != timings.begin())
  {
    std::map<std::string,TIMING_DATA>::iterator it;
    std::string fmt;
    uint_t maxlen = 0;

    Printf(res, "Performance summary:\n");

    // find maximum ID string length
    for (it = timings.begin(); it != timings.end(); ++it)
    {
      const std::string& id = it->first;

      maxlen = MAX(maxlen, id.length());
    }

    // create format string (with ID length fround above)
    // (the double-percents get converted to single percents to be used as formatting strings in the Printf statement below)
    // (apart from the quad-percents which get converted to double-percents which then become single percents in the final string, i.e. the percent sign!)
    Printf(fmt, "%%3u: '%%-%us' taken %%14.9lfs of elapsed %%14.9lfs utilization %%5.1lf%%%% (min %%5.1lf%%%% max %%5.1lf%%%%)\n", maxlen);

    // output data for each entry
    for (it = timings.begin(); it != timings.end(); ++it)
    {
      const std::string& id = it->first;
      TIMING_DATA& data     = it->second;

      Printf(res, fmt.c_str(), data.instance, id.c_str(), DISP(data.total_taken), DISP(data.total_elapsed), 100.0 * (double)data.total_taken / (double)data.total_elapsed, data.min_utilization, data.max_utilization);
    }
  }

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Return textual performance report (static wrapper for the above)
 */
/*--------------------------------------------------------------------------------*/
std::string PerformanceMonitor::GetReport()
{
  return Get().GetReportEx();
}

/*--------------------------------------------------------------------------------*/
/** Enable measuring
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::StartMeasuring()
{
  Get().measure = true;
}

/*--------------------------------------------------------------------------------*/
/** Disable measuring
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::StopMeasuring()
{
  Get().measure = false;
}

/*--------------------------------------------------------------------------------*/
/** Start logging to file
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::StartLogging()
{
  Get().logtofile = true;
}

/*--------------------------------------------------------------------------------*/
/** Stop logging to file
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::StopLogging()
{
  Get().logtofile = false;
}

/*--------------------------------------------------------------------------------*/
/** Start logging to individual files
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::StartIndividualLogging()
{
  Get().logtofiles = true;
}

/*--------------------------------------------------------------------------------*/
/** Stop logging to individual files
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::StopIndividualLogging()
{
  Get().logtofiles = false;
}

/*--------------------------------------------------------------------------------*/
/** Enable performance report at destruction
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::EnablePerformanceReport(bool enable)
{
  Get().reportatend = enable;
}

PerformanceMonitor::perftime_t PerformanceMonitor::GetCurrent()
{
#ifdef __MACH__
  static mach_timebase_info_data_t timebase;
  static bool inited = false;

  if (!inited)
  {
    mach_timebase_info(&timebase);
    inited = true;
  }

  uint64_t tick = mach_absolute_time();
  tick = (tick * timebase.numer) / timebase.denom;

  perftime_t t = tick;
#else
  struct timespec timespec;

#ifdef ANDROID
  clock_gettime(CLOCK_MONOTONIC_HR, &timespec);
#elif defined(__CYGWIN__)
  clock_gettime(CLOCK_MONOTONIC, &timespec);
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
#endif

  perftime_t t = (perftime_t)timespec.tv_sec * (perftime_t)1000000000 + (perftime_t)timespec.tv_nsec;
#endif

  if (!t0) t0 = t;

  return t - t0;
}

void PerformanceMonitor::LogToFile(FILE *fp, perftime_t t, const TIMING_DATA& data, const std::string& id, bool start) const
{
  if (fp)
  {
    const TIMING& this_timing = data.timings[data.index];
    const TIMING& last_timing = data.timings[(data.index + data.ntimings - 1) % data.ntimings];

    if (ftell(fp) == 0)
    {
      fprintf(fp, "Time Start/Stop \"Start Time\" \"Stop Time\" \"Average Elapsed\" \"Average Taken\" \"This Elapsed\" \"This Taken\" \"Last Start/Stop\" Utilization Instance ID\n");
    }

    fprintf(fp, "%0.9lf %2d %0.9lf %0.9lf %0.9lf %0.9lf %0.9lf %0.9lf %0.9lf %0.3lf %u \"%s (%s)\"\n",
            DISP(t),
            start ? 1 : -1,
            DISP(this_timing.start),
            DISP(start ? last_timing.stop : this_timing.stop),
            DISP(data.elapsed),
            DISP(data.taken),
            DISP(this_timing.elapsed),
            DISP(start ? last_timing.taken : this_timing.taken),
            DISP(start ? last_timing.start : last_timing.stop),
            data.utilization,
            data.instance,
            id.c_str(),
            start ? "Start" : "Stop");
  }
}

/*--------------------------------------------------------------------------------*/
/** Start performance measurement
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::Start(const std::string& id)
{
  // abort quickly if measurement is not enabled
  if (!measure) return;

  ThreadLock lock(tlock);
  std::map<std::string,TIMING_DATA>::iterator it;

  if ((it = timings.find(id)) == timings.end())
  {
    TIMING_DATA timing;

    memset(&timing, 0, sizeof(timing));

    timing.instance = timings.size();
    timing.ntimings = avglen;
    if ((timing.timings = new TIMING[timing.ntimings]) != NULL)
    {
      memset(timing.timings, 0, timing.ntimings * sizeof(*timing.timings));

      timings[id] = timing;
      it = timings.find(id);
    }

    DEBUG3(("Creating timing data for '%s'", id.c_str()));
  }

  if (it != timings.end())
  {
    TIMING_DATA& data   = it->second;
    TIMING&      timing = data.timings[data.index];
    perftime_t   t = GetCurrent();

    // remove old elapsed value from running average
    data.elapsed -= timing.elapsed;
    // calculate new elapsed value
    timing.start   = t;
    timing.elapsed = t - data.timings[(data.index + data.ntimings - 1) % data.ntimings].start;
    // add new elapsed value to running average
    data.elapsed += timing.elapsed;
    // update total
    data.total_elapsed += timing.elapsed;
    // update max/min
    data.max_elapsed = MAX(data.max_elapsed, timing.elapsed);
    if (!data.wrapped && (data.index == 0)) data.min_elapsed = timing.elapsed;
    else                                    data.min_elapsed = MIN(data.min_elapsed, timing.elapsed);

    // update utilization values
    perftime_t last_taken = data.timings[(data.index + data.ntimings - 1) % data.ntimings].taken;
    double ut = timing.elapsed ? 100.0 * (double)last_taken / (double)timing.elapsed : 0.0;
    data.utilization = ut;
    data.max_utilization = MAX(data.max_utilization, ut);
    if (!data.wrapped && (data.index == 1)) data.min_utilization = ut;
    else                                    data.min_utilization = MIN(data.min_utilization, ut);

    if (logtofile)
    {
      if (!fp) fp = fopen("perfdata.dat", "w");

      LogToFile(fp, t, data, id, true);
    }

    if (logtofiles)
    {
      if (!data.fp)
      {
        std::string filename;

        Printf(filename, "perf-%u.dat", data.instance);
        data.fp = fopen(filename.c_str(), "w");
      }

      LogToFile(data.fp, t, data, id, true);
    }
  }
  else ERROR("No timing data for ID '%s'", id.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Stop performance measurement
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::Stop(const std::string& id)
{
  // abort quickly if measurement is not enabled
  if (!measure) return;

  ThreadLock lock(tlock);
  std::map<std::string,TIMING_DATA>::iterator it;

  if ((it = timings.find(id)) != timings.end())
  {
    TIMING_DATA& data   = it->second;
    TIMING&      timing = data.timings[data.index];
    perftime_t   t = GetCurrent();

    // remove old taken value from running average
    data.taken -= timing.taken;
    // calculate new taken value
    timing.stop  = t;
    timing.taken = t - timing.start;
    // add new taken value to running average
    data.taken += timing.taken;
    // update total
    data.total_taken += timing.taken;
    // update max/min
    data.max_taken = MAX(data.max_taken, timing.taken);
    if (!data.wrapped && (data.index == 0)) data.min_taken = timing.taken;
    else                                    data.min_taken = MIN(data.min_taken, timing.taken);

    if (logtofile)
    {
      LogToFile(fp, t, data, id, false);
    }

    if (logtofiles)
    {
      LogToFile(data.fp, t, data, id, false);
    }

    // detect wrap-around and buffer as wrapped (for full running averages)
    if ((++data.index) == data.ntimings)
    {
      data.wrapped = true;
      data.index   = 0;
    }
  }
  else ERROR("No timing data for ID '%s'", id.c_str());
}

BBC_AUDIOTOOLBOX_END
