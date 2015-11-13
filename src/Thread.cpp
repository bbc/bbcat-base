
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
                             threadcompleted(false)
{
  // clear thread data
  memset(&thread, 0, sizeof(thread));

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
                                               threadcompleted(false)
{
  // clear thread data
  memset(&thread, 0, sizeof(thread));

  if (_call) Start(_call, _arg);
}

Thread::~Thread()
{
  Stop();
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
    stopthread = abortthread = threadcompleted = false;
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
  }

  return started;
}

/*--------------------------------------------------------------------------------*/
/** Return whether thread is running or not
 */
/*--------------------------------------------------------------------------------*/
bool Thread::IsRunning() const
{
#ifdef COMPILER_MSVC
  return (thread.p != NULL);
#else
  return (thread != 0);
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

    // if thread has completed, force join
    wait |= HasCompleted();

    if (wait)
    {
      pthread_join(thread, NULL);
      memset(&thread, 0, sizeof(thread));
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
/** Main thread entry point
 */
/*--------------------------------------------------------------------------------*/
void *Thread::Run()
{
  void *res = NULL;
  
  // if callback defined, call it
  if (call) res = (*call)(*this, arg);

  if (!abortthread) Complete();

  return res;
}

BBC_AUDIOTOOLBOX_END
