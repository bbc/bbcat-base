
#include <string.h>

#include <errno.h>

#define BBCDEBUG_LEVEL 1
#include "misc.h"
#include "ThreadLock.h"

BBC_AUDIOTOOLBOX_START

ThreadLockObject::ThreadLockObject()
#ifndef USE_PTHREADS
  : lock(mutex, std::defer_lock)
#endif
{
#ifdef USE_PTHREADS
  pthread_mutexattr_t mta;

  pthread_mutexattr_init(&mta);
  /* or PTHREAD_MUTEX_RECURSIVE_NP */
  pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);     // MUST be recursive!

  if (pthread_mutex_init(&mutex, &mta) != 0)
  {
    BBCERROR("Failed to initialise mutex<%s>: %s", StringFrom(&mutex).c_str(), strerror(errno));
  }
#endif
}

ThreadLockObject::~ThreadLockObject()
{
#ifdef USE_PTHREADS
  pthread_mutex_destroy(&mutex);
#endif
}

bool ThreadLockObject::Lock()
{
#ifdef USE_PTHREADS
  bool success = (pthread_mutex_lock(&mutex) == 0);

  if (!success) BBCERROR("Failed to lock mutex<%s>: %s", StringFrom(&mutex).c_str(), strerror(errno));

  return success;
#else
  lock.lock();
  return true;
#endif
}

bool ThreadLockObject::Unlock()
{
#ifdef USE_PTHREADS
  bool success = (pthread_mutex_unlock(&mutex) == 0);

  if (!success) BBCERROR("Failed to unlock mutex<%s>: %s", StringFrom(&mutex).c_str(), strerror(errno));

  return success;
#else
  lock.unlock();
  return true;
#endif
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Constructor locks ThreadLockObject
 */
/*--------------------------------------------------------------------------------*/
ThreadLock::ThreadLock(ThreadLockObject& lockobj) :
#ifdef USE_PTHREADS
  obj(lockobj)
#else
  guard(lockobj.mutex)
#endif
{
#ifdef USE_PTHREADS
  obj.Lock();
#endif
}

/*--------------------------------------------------------------------------------*/
/** Const constructor to allow use in const methods
 */
/*--------------------------------------------------------------------------------*/
ThreadLock::ThreadLock(const ThreadLockObject& lockobj) :
#ifdef USE_PTHREADS
  obj(const_cast<ThreadLockObject&>(lockobj))
#else
  guard(const_cast<ThreadLockObject&>(lockobj).mutex)
#endif
{
#ifdef USE_PTHREADS
  obj.Lock();
#endif
}

/*--------------------------------------------------------------------------------*/
/** Destructor unlocks ThreadLockObject
 */
/*--------------------------------------------------------------------------------*/
ThreadLock::~ThreadLock()
{
#ifdef USE_PTHREADS
  obj.Unlock();
#endif
}

/*----------------------------------------------------------------------------------------------------*/

ThreadSignalObject::ThreadSignalObject()
#ifdef USE_PTHREADS
  : ThreadLockObject()
#endif
{
#ifdef USE_PTHREADS
  if (pthread_cond_init(&condition, NULL) != 0)
  {
    BBCERROR("Failed to initialise cond<%s>: %s", StringFrom(&condition).c_str(), strerror(errno));
  }
#endif
}

ThreadSignalObject::~ThreadSignalObject()
{
}

bool ThreadSignalObject::Wait()
{
  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Waiting on condition<%s> pre-lock", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

#ifdef USE_PTHREADS
  ThreadLock lock(*this);
#else
  std::unique_lock<std::mutex> lock(mutex);
#endif
  bool success = true;

  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Waiting on condition<%s> post-lock", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));
  
  while (!IsReady())
  {
    BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Waiting on condition<%s> waiting", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

#ifdef USE_PTHREADS
    success = (pthread_cond_wait(&condition, &mutex) == 0);
    if (!success)
    {
      BBCERROR("Failed to wait on cond<%s>: %s", StringFrom(&condition).c_str(), strerror(errno));
      break;
    }
#else
    condition.wait(lock);
#endif

    BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Waiting on condition<%s> waiting complete, %sready", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str(), IsReady() ? "" : "NOT "));
  }

  ClearReady();

  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Waiting on condition<%s> complete", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

  return success;
}

bool ThreadSignalObject::Signal()
{
  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Signal condition<%s> pre-lock", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

#ifdef USE_PTHREADS
  ThreadLock lock(*this);
#else
  std::unique_lock<std::mutex> lock(mutex);
#endif
  bool success = true;
  
  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Signal condition<%s> post-lock", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

  SetReady();
  
#ifdef USE_PTHREADS
  success = (pthread_cond_signal(&condition) == 0);
  if (!success) BBCERROR("Failed to signal cond<%s>: %s", StringFrom(&condition).c_str(), strerror(errno));
#else
  condition.notify_one();
#endif

  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Signal condition<%s> complete", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

  return success;
}

bool ThreadSignalObject::Broadcast()
{
  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Broadcast condition<%s> pre-lock", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

#ifdef USE_PTHREADS
  ThreadLock lock(*this);
#else
  std::unique_lock<std::mutex> lock(mutex);
#endif
  bool success = true;

  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Broadcast condition<%s> post-lock", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));

  SetReady();
  
#ifdef USE_PTHREADS
  success = (pthread_cond_broadcast(&condition) == 0);
  if (!success) BBCERROR("Failed to signal cond<%s>: %s", StringFrom(&condition).c_str(), strerror(errno));
#else
  condition.notify_all();
#endif

  BBCDEBUG2(("[%s] ThreadSignalObject<%s>: Broadcast condition<%s> complete", StringFrom(GetTickCount(), "010").c_str(), StringFrom(this).c_str(), StringFrom(&condition).c_str()));
  
  return success;
}

/*----------------------------------------------------------------------------------------------------*/

ThreadBoolSignalObject::ThreadBoolSignalObject(bool initial_condition) : ThreadSignalObject(),
                                                                         ready(initial_condition)
{
}

ThreadBoolSignalObject::~ThreadBoolSignalObject()
{
}

BBC_AUDIOTOOLBOX_END
