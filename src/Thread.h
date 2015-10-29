#ifndef __THREAD__
#define __THREAD__

#ifdef _MSC_VER
// prevent MSDev throwing a wobbly with struct timespec defined in pthread.h
#include <time.h>
#define HAVE_STRUCT_TIMESPEC
#endif

#include <pthread.h>

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
  virtual ~Thread();

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
  /** Return whether thread has completed
   */
  /*--------------------------------------------------------------------------------*/
  bool HasCompleted() const {return threadcompleted;}

protected:
  static void *__ThreadEntry(void *arg)
  {
    return ((Thread *)arg)->Run();
  }

  /*--------------------------------------------------------------------------------*/
  /** Main thread entry point
   */
  /*--------------------------------------------------------------------------------*/
  virtual void *Run();

protected:
  pthread_t  thread;
  THREADCALL call;
  void       *arg;
  bool       stopthread;
  bool       abortthread;
  bool       threadcompleted;
};

BBC_AUDIOTOOLBOX_END

#endif
