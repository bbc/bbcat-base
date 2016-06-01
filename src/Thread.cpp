
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define BBCDEBUG_LEVEL 1
#include "Thread.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Default constructor - can start derived class thread
 */
/*--------------------------------------------------------------------------------*/
Thread::Thread(bool start) : call(NULL),
                             arg(NULL),
                             stopthread(false),
                             abortthread(false),
                             threadcompleted(false),
                             threadfinished(false)
{
#ifdef USE_PTHREADS
  // clear thread data
  memset(&thread, 0, sizeof(thread));
#endif
  
  if (start) Start();
}

/*--------------------------------------------------------------------------------*/
/** Callback constructor - automatically starts callback thread
 */
/*--------------------------------------------------------------------------------*/
Thread::Thread(THREADCALL _call, void *_arg) : call(NULL),
                                               arg(NULL),
                                               stopthread(false),
                                               abortthread(false),
                                               threadcompleted(false),
                                               threadfinished(false)
{
#ifdef USE_PTHREADS
  // clear thread data
  memset(&thread, 0, sizeof(thread));
#endif

  if (_call) Start(_call, _arg);
}

Thread::~Thread()
{
  Stop();
}

/*--------------------------------------------------------------------------------*/
/** Copy constructor
 */
/*--------------------------------------------------------------------------------*/
Thread::Thread(const Thread& obj) : call(NULL),
                                    arg(NULL),
                                    stopthread(false),
                                    abortthread(false),
                                    threadcompleted(false)
{
#ifdef USE_PTHREADS
  // clear thread data
  memset(&thread, 0, sizeof(thread));
#endif

  operator = (obj);
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
Thread& Thread::operator = (const Thread& obj)
{
  if (&obj != this)
  {
    assert(!IsRunning());
    assert(!obj.IsRunning());
    
    call = obj.call;
    arg  = obj.arg;
    stopthread = abortthread = threadcompleted = threadfinished = false;
  }
  
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Start callback thread
 */
/*--------------------------------------------------------------------------------*/
bool Thread::Start(THREADCALL _call, void *_arg)
{
  bool started = false;

  if (!IsRunning())
  {
    call    = _call;
    arg     = _arg; 
    started = Start();
  }

  return started;
}

/*--------------------------------------------------------------------------------*/
/** Start thread (derived or callback)
 */
/*--------------------------------------------------------------------------------*/
bool Thread::Start()
{
  bool started = false;

  if (!IsRunning())
  {
    // create thread
    stopthread = abortthread = threadcompleted = threadfinished = false;

#ifdef USE_PTHREADS
    if (pthread_create(&thread, NULL, &__ThreadEntry, (void *)this) == 0)
    {
      BBCDEBUG2(("Created thread"));
      started = true;
    }
    else
    {
      BBCERROR("Failed to create thread (%s)", strerror(errno));
      memset(&thread, 0, sizeof(thread));
    }
#else
    thread = std::thread( [this]() {
        try {
          Run();
        }
        catch (std::exception e)
        {
          BBCERROR("Exception in ThreadSTL: %s", e.what());
        }
      });
    started = true;
#endif
  }

  return started;
}

/*--------------------------------------------------------------------------------*/
/** Return whether thread is running or not
 */
/*--------------------------------------------------------------------------------*/
bool Thread::IsRunning() const
{
#ifdef USE_PTHREADS

#ifdef COMPILER_MSVC
  return (thread.p != NULL);
#else
  return (thread != 0);
#endif

#else
  return thread.joinable();
#endif
}

/*--------------------------------------------------------------------------------*/
/** Request thread stop and optionally wait until is has finished
 */
/*--------------------------------------------------------------------------------*/
void Thread::Stop(bool wait)
{
  if (IsRunning())
  {
    stopthread = true;

    // if thread has finished, force join
    wait |= HasFinished();

    if (wait)
    {
#ifdef USE_PTHREADS
      pthread_join(thread, NULL);
      memset(&thread, 0, sizeof(thread));
#else
      thread.join();
#endif
      
      stopthread = abortthread = false;
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Request thread *abort* and optionally wait until is has finished
 */
/*--------------------------------------------------------------------------------*/
void Thread::Abort(bool wait)
{
  if (IsRunning())
  {
    abortthread = stopthread = true;

    Stop(wait);
  }
}

/*--------------------------------------------------------------------------------*/
/** Main thread entry point (non-overridable)
 */
/*--------------------------------------------------------------------------------*/
void *Thread::RunEx()
{
  void *res;
  
  res = Run();

  // if not aborted, mark as completed
  if (!AbortRequested()) Complete();

  // whatever happens, mark as finished
  Finished();
  
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Overridable thread routine
 */
/*--------------------------------------------------------------------------------*/
void *Thread::Run()
{
  void *res = NULL;
  
  // if callback defined, call it
  if (call) res = (*call)(*this, arg);

  return res;
}

BBC_AUDIOTOOLBOX_END
