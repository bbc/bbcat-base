#ifndef __PERFORMANCE_MONITOR__
#define __PERFORMANCE_MONITOR__

#include <stdarg.h>

#include <string>
#include <map>

#include "misc.h"
#include "ThreadLock.h"

BBC_AUDIOTOOLBOX_START

// unless specified as enabled or disabled before this include, allow use of PERFMON() macro
#ifndef PERFORMANCE_MONITORING_ENABLED
#define PERFORMANCE_MONITORING_ENABLED 1
#endif

/*--------------------------------------------------------------------------------*/
/** Simple averaging performance monitor
 *
 * Not to be used directly but instead used by PerformanceMonitorMarker class and PERFMON macro
 */
/*--------------------------------------------------------------------------------*/
class PerformanceMonitor
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Get access to Performance Monitor singleton
   */
  /*--------------------------------------------------------------------------------*/
  static PerformanceMonitor& Get();

  /*--------------------------------------------------------------------------------*/
  /** Enable measuring
   */
  /*--------------------------------------------------------------------------------*/
  static void StartMeasuring();

  /*--------------------------------------------------------------------------------*/
  /** Disable measuring
   */
  /*--------------------------------------------------------------------------------*/
  static void StopMeasuring();

  /*--------------------------------------------------------------------------------*/
  /** Start logging to file
   */
  /*--------------------------------------------------------------------------------*/
  static void StartLogging();

  /*--------------------------------------------------------------------------------*/
  /** Stop logging to file
   */
  /*--------------------------------------------------------------------------------*/
  static void StopLogging();

  /*--------------------------------------------------------------------------------*/
  /** Start logging to individual files
   */
  /*--------------------------------------------------------------------------------*/
  static void StartIndividualLogging();

  /*--------------------------------------------------------------------------------*/
  /** Stop logging to individual files
   */
  /*--------------------------------------------------------------------------------*/
  static void StopIndividualLogging();

  /*--------------------------------------------------------------------------------*/
  /** Enable performance report at destruction
   */
  /*--------------------------------------------------------------------------------*/
  static void EnablePerformanceReport(bool enable = true);

  /*--------------------------------------------------------------------------------*/
  /** Enable production of GNUPlot file (forces individual logging files)
   */
  /*--------------------------------------------------------------------------------*/
  static void EnableGNUPlotFile(bool enable = true);

  /*--------------------------------------------------------------------------------*/
  /** Start performance measurement
   */
  /*--------------------------------------------------------------------------------*/
  void Start(const std::string& id);

  /*--------------------------------------------------------------------------------*/
  /** Stop performance measurement
   */
  /*--------------------------------------------------------------------------------*/
  void Stop(const std::string& id);

  /*--------------------------------------------------------------------------------*/
  /** Return textual performance report
   */
  /*--------------------------------------------------------------------------------*/
  static std::string GetReport();

private:
  PerformanceMonitor(uint_t _avglen = 10);
  ~PerformanceMonitor();

protected:
  typedef uint64_t perftime_t;

  perftime_t GetCurrent();

  typedef struct
  {
    perftime_t  start;
    perftime_t  stop;
    perftime_t  elapsed;
    perftime_t  taken;
  } TIMING;

  typedef struct
  {
    std::string id;
    struct {
      FILE      *fp;
      uint_t    instance;
    } config;

    TIMING      *timings;
    uint_t      ntimings, index;
    bool        wrapped;

    struct {
      perftime_t  elapsed;
      perftime_t  taken;
      perftime_t  total_elapsed;
      perftime_t  total_taken;
      perftime_t  max_elapsed;
      perftime_t  max_taken;
      perftime_t  min_elapsed;
      perftime_t  min_taken;
      double      utilization;
      double      max_utilization;
      double      min_utilization;
    } stats;
  } TIMING_DATA;

  void LogToFile(FILE *fp, perftime_t t, const TIMING_DATA& data, const std::string& id, bool start) const;

  /*--------------------------------------------------------------------------------*/
  /** Return textual performance report
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetReportEx();
  
protected:
  ThreadLockObject tlock;
  perftime_t       t0;
  uint_t           avglen;
  std::map<std::string,TIMING_DATA> timings;
  std::vector<const TIMING_DATA *>  timingslist;
  std::string      logfiledir;
  
  FILE *fp;
  bool measure;
  bool logtofile;
  bool logtofiles;
  bool reportatend;
  bool generategnuplotfile;
};

/*--------------------------------------------------------------------------------*/
/** Simple class wrapper for performance start/stop
 */
/*--------------------------------------------------------------------------------*/
class PerformanceMonitorMarker
{
public:
  PerformanceMonitorMarker(const char *_id) : id(_id) {PerformanceMonitor::Get().Start(id);}
  ~PerformanceMonitorMarker() {PerformanceMonitor::Get().Stop(id);}

protected:
  std::string id;
};

#if PERFORMANCE_MONITORING_ENABLED
/*--------------------------------------------------------------------------------*/
/** Macro for monitoring which allows flexible naming
 */
/*--------------------------------------------------------------------------------*/
#define PERFMON(id) PerformanceMonitorMarker _mon(StringStream() << id)
#else
// disable macro -> disable monitoring
#define PERFMON(id) (void)0
#endif

BBC_AUDIOTOOLBOX_END

#endif
