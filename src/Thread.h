#ifndef __THREAD_H__
#define __THREAD_H__

#ifdef USE_PTHREADS
#include <pthread.h>
#else
#include <thread>
#include <type_traits>
#include <atomic>
#endif

// include early for section below
#include "OSCompiler.h"

#ifdef COMPILER_MSVC
// prevent MSDev throwing a wobbly with struct timespec defined in pthread.h
#include <time.h>
#define HAVE_STRUCT_TIMESPEC
#endif

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Simple class representing a thread
 *
 * This can either be derived from (and overriding Run()) or can use a callback mechanism
 */
/*--------------------------------------------------------------------------------*/
class Thread
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Definition of thread callback routine
   */
  /*--------------------------------------------------------------------------------*/
  typedef void *(*THREADCALL)(Thread& thread, void *arg);

  /*--------------------------------------------------------------------------------*/
  /** Default constructor - can start derived class thread
   */
  /*--------------------------------------------------------------------------------*/
  Thread(bool start = false);

  /*--------------------------------------------------------------------------------*/
  /** Callback constructor - automatically starts callback thread
   */
  /*--------------------------------------------------------------------------------*/
  Thread(THREADCALL _call, void *_arg = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Copy constructor
   */
  /*--------------------------------------------------------------------------------*/
  Thread(const Thread& obj);

  virtual ~Thread();

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  Thread& operator = (const Thread& obj);

  /*--------------------------------------------------------------------------------*/
  /** Start callback thread
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Start(THREADCALL _call, void *_arg = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Start thread (derived or callback)
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Start();

  /*--------------------------------------------------------------------------------*/
  /** Return whether thread is running or not
   */
  /*--------------------------------------------------------------------------------*/
  bool IsRunning() const;

  /*--------------------------------------------------------------------------------*/
  /** Request thread stop and optionally wait until is has finished
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Stop(bool wait = true);

  /*--------------------------------------------------------------------------------*/
  /** Request thread *abort* and optionally wait until is has finished
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Abort(bool wait = true);

  /*--------------------------------------------------------------------------------*/
  /** Return whether thread has been requested to stop
   */
  /*--------------------------------------------------------------------------------*/
  bool StopRequested() const {return stopthread;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether thread has been requested to abort
   */
  /*--------------------------------------------------------------------------------*/
  bool AbortRequested() const {return abortthread;}

  /*--------------------------------------------------------------------------------*/
  /** Mark thread as completed
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Complete() {threadcompleted = true;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether thread has completed successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool HasCompleted() const {return threadcompleted;}

  /*--------------------------------------------------------------------------------*/
  /** Mark thread as finished
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Finished() {threadfinished = true;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether thread has finished (completed or aborted)
   */
  /*--------------------------------------------------------------------------------*/
  bool HasFinished() const {return threadfinished;}

protected:
  static void *__ThreadEntry(void *arg)
  {
    return ((Thread *)arg)->RunEx();
  }

  /*--------------------------------------------------------------------------------*/
  /** Main thread entry point (non-overridable)
   */
  /*--------------------------------------------------------------------------------*/
  void *RunEx();

  /*--------------------------------------------------------------------------------*/
  /** Overridable thread routine
   */
  /*--------------------------------------------------------------------------------*/
  virtual void *Run();

protected:
#ifdef USE_PTHREADS
  pthread_t  thread;
#else
  // TLDR Using a thread rather than async/future to avoid hang on exit
  // when used in plugins

  // When a windows process loads a dll then exits rather than unloading dll
  // dll-launched threads are killed before static destructors run
  //
  // An async future's destructor will block on completion of the
  // async function (if launched via async policy). If the underlying thread
  // is already dead, it blocks forever (VS2013).
  //
  // So if you've a statically defined renderer in a windows dll & the parent process doesn't unload, it hangs on exit
  //
  // OTOH std::thread seems to tolerate a join to a dead thread.

  std::thread thread;
#endif
  THREADCALL call;
  void       *arg;
  bool       stopthread;
  bool       abortthread;
  bool       threadcompleted;
  bool       threadfinished;
};

BBC_AUDIOTOOLBOX_END

#endif
