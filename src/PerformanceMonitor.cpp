
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <string>

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

#define DEBUG_LEVEL 2
#include "EnhancedFile.h"
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
  reportatend(REPORT_PERFORMANCE_BY_DEFAULT),
  generategnuplotfile(false)
{
}

PerformanceMonitor::~PerformanceMonitor()
{
  ThreadLock lock(tlock);
  std::map<std::string,TIMING_DATA>::iterator it;

  if (fp) fclose(fp);

  for (it = timings.begin(); it != timings.end(); ++it)
  {
    TIMING_DATA& data = it->second;

    if (data.config.fp) fclose(data.config.fp);
  }

  if (reportatend)
  {
    std::string res = GetReportEx();

    DEBUG("%s", res.c_str());
  }

  if (generategnuplotfile)
  {
    EnhancedFile file;

    if (file.fopen("plot.gnp", "w"))
    {
      uint_t i;

      file.fprintf("splot \\\n");

      for (i = 0; i < timingslist.size(); i++)
      {
        file.fprintf("  'perf-%u.dat' using 1:($11+.25*$2):(($2<0)?$8:0) with linespoints title '%s (%u)', \\\n", i, timingslist[i]->id.c_str(), i);
      }

      file.fprintf("  0 lt 0\n");
      file.fclose();
    }
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

  if (timings.size())
  {
    std::string fmt;
    uint_t i, maxlen = 0;

    Printf(res, "Performance summary:\n");

    // find maximum ID string length
    for (i = 0; i < timings.size(); i++)
    {
      const std::string& id = timingslist[i]->id;

      maxlen = MAX(maxlen, (uint_t)id.length());
    }

    // create format string (with ID length fround above)
    // (the double-percents get converted to single percents to be used as formatting strings in the Printf statement below)
    // (apart from the quad-percents which get converted to double-percents which then become single percents in the final string, i.e. the percent sign!)
    Printf(fmt, "%%3u: '%%-%us' taken %%14.9lfs of elapsed %%14.9lfs utilization %%5.1lf%%%% (min %%5.1lf%%%% max %%5.1lf%%%%)\n", maxlen);

    // output data for each entry
    for (i = 0; i < timings.size(); i++)
    {
      const std::string& id   = timingslist[i]->id;
      const TIMING_DATA& data = *timingslist[i];

      Printf(res,
             fmt.c_str(),
             data.config.instance,
             id.c_str(),
             DISP(data.stats.total_taken),
             DISP(data.stats.total_elapsed),
             100.0 * (double)data.stats.total_taken / (double)data.stats.total_elapsed,
             data.stats.min_utilization,
             data.stats.max_utilization);
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

/*--------------------------------------------------------------------------------*/
/** Enable production of GNUPlot file (forces individual logging files)
 */
/*--------------------------------------------------------------------------------*/
void PerformanceMonitor::EnableGNUPlotFile(bool enable)
{
  Get().generategnuplotfile = enable;
  Get().logtofiles         |= enable;
}

PerformanceMonitor::perftime_t PerformanceMonitor::GetCurrent()
{
  perftime_t t = (perftime_t)GetNanosecondTicks();

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
      fprintf(fp, "Time Start/Stop \"Start Time\" \"Stop Time\" \"Average Elapsed\" \"Average Taken\" \"This Elapsed\" \"This Taken\" \"Last Start/Stop\" Utilization Instance ID Thread\n");
    }

#ifdef TARGET_OS_WINDOWS
    const void *self = pthread_self().p;
#else
    const void *self = (const void *)pthread_self();
#endif
    
    fprintf(fp, "%0.9lf %2d %0.9lf %0.9lf %0.9lf %0.9lf %0.9lf %0.9lf %0.9lf %0.3lf %u \"%s (%s)\" \"Thread<%s>\"\n",
            DISP(t),
            start ? 1 : -1,
            DISP(this_timing.start),
            DISP(start ? last_timing.stop : this_timing.stop),
            DISP(data.stats.elapsed),
            DISP(data.stats.taken),
            DISP(this_timing.elapsed),
            DISP(start ? last_timing.taken : this_timing.taken),
            DISP(start ? last_timing.start : last_timing.stop),
            data.stats.utilization,
            data.config.instance,
            id.c_str(),
            start ? "Start" : "Stop",
            StringFrom(self).c_str());
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

    timing.id = id;

    memset(&timing.config, 0, sizeof(timing.config));
    memset(&timing.stats,  0, sizeof(timing.stats));

    timing.config.instance = (uint_t)timings.size();

    timing.index    = 0;
    timing.wrapped  = false;
    timing.ntimings = avglen;
    if ((timing.timings = new TIMING[timing.ntimings]) != NULL)
    {
      memset(timing.timings, 0, timing.ntimings * sizeof(*timing.timings));

      timings[id] = timing;
      timingslist.push_back(&timings[id]);
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
    data.stats.elapsed -= timing.elapsed;
    // calculate new elapsed value
    timing.start   = t;
    timing.elapsed = t - data.timings[(data.index + data.ntimings - 1) % data.ntimings].start;
    // add new elapsed value to running average
    data.stats.elapsed += timing.elapsed;
    // update total
    data.stats.total_elapsed += timing.elapsed;
    // update max/min
    data.stats.max_elapsed = MAX(data.stats.max_elapsed, timing.elapsed);
    if (!data.wrapped && (data.index == 0)) data.stats.min_elapsed = timing.elapsed;
    else                                    data.stats.min_elapsed = MIN(data.stats.min_elapsed, timing.elapsed);

    // update utilization values
    perftime_t last_taken = data.timings[(data.index + data.ntimings - 1) % data.ntimings].taken;
    double ut = timing.elapsed ? 100.0 * (double)last_taken / (double)timing.elapsed : 0.0;
    data.stats.utilization = ut;
    data.stats.max_utilization = MAX(data.stats.max_utilization, ut);
    if (!data.wrapped && (data.index == 1)) data.stats.min_utilization = ut;
    else                                    data.stats.min_utilization = MIN(data.stats.min_utilization, ut);

    if (logtofile)
    {
      if (!fp) fp = fopen("perfdata.dat", "w");

      LogToFile(fp, t, data, id, true);
    }

    if (logtofiles)
    {
      if (!data.config.fp)
      {
        std::string filename;

        Printf(filename, "perf-%u.dat", data.config.instance);
        data.config.fp = fopen(filename.c_str(), "w");
      }

      LogToFile(data.config.fp, t, data, id, true);
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
    data.stats.taken -= timing.taken;
    // calculate new taken value
    timing.stop  = t;
    timing.taken = t - timing.start;
    // add new taken value to running average
    data.stats.taken += timing.taken;
    // update total
    data.stats.total_taken += timing.taken;
    // update max/min
    data.stats.max_taken = MAX(data.stats.max_taken, timing.taken);
    if (!data.wrapped && (data.index == 0)) data.stats.min_taken = timing.taken;
    else                                    data.stats.min_taken = MIN(data.stats.min_taken, timing.taken);

    if (logtofile)
    {
      LogToFile(fp, t, data, id, false);
    }

    if (logtofiles)
    {
      LogToFile(data.config.fp, t, data, id, false);
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
