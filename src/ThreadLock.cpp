
#include <string.h>

#include <errno.h>

#define DEBUG_LEVEL 1
#include "misc.h"
#include "ThreadLock.h"

BBC_AUDIOTOOLBOX_START

ThreadLockObject::ThreadLockObject()
{
  pthread_mutexattr_t mta;

  pthread_mutexattr_init(&mta);
  /* or PTHREAD_MUTEX_RECURSIVE_NP */
  pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);     // MUST be recursive!

  if (pthread_mutex_init(&mutex, &mta) != 0)
  {
    ERROR("Failed to initialise mutex<%016lx>: %s", (ulong_t)&mutex, strerror(errno));
  }
}

ThreadLockObject::~ThreadLockObject()
{
  pthread_mutex_destroy(&mutex);
}

bool ThreadLockObject::Lock()
{
  bool success = (pthread_mutex_lock(&mutex) == 0);

  if (!success) ERROR("Failed to lock mutex<%016lx>: %s", (ulong_t)&mutex, strerror(errno));

  return success;
}

bool ThreadLockObject::Unlock()
{
  bool success = (pthread_mutex_unlock(&mutex) == 0);

  if (!success) ERROR("Failed to unlock mutex<%016lx>: %s", (ulong_t)&mutex, strerror(errno));

  return success;
}

/*----------------------------------------------------------------------------------------------------*/

ThreadSignalObject::ThreadSignalObject() : ThreadLockObject()
{
  if (pthread_cond_init(&cond, NULL) != 0)
  {
    ERROR("Failed to initialise cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));
  }
}

ThreadSignalObject::~ThreadSignalObject()
{
  pthread_cond_destroy(&cond);
}

bool ThreadSignalObject::Wait()
{
  // NOTE: mutex MUST be LOCKED at this point
  bool success = (pthread_cond_wait(&cond, &mutex) == 0);

  if (!success) ERROR("Failed to wait on cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));

  return success;
}

bool ThreadSignalObject::Signal()
{
  ThreadLock lock(*this);
  bool success = (pthread_cond_signal(&cond) == 0);

  if (!success) ERROR("Failed to signal cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));

  return success;
}

bool ThreadSignalObject::Broadcast()
{
  ThreadLock lock(*this);
  bool success = (pthread_cond_broadcast(&cond) == 0);

  if (!success) ERROR("Failed to broadcast to cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));

  return success;
}

/*----------------------------------------------------------------------------------------------------*/

ThreadBoolSignalObject::ThreadBoolSignalObject(bool initial_condition) : ThreadSignalObject(),
                                                                         condition(initial_condition)
{
}

ThreadBoolSignalObject::~ThreadBoolSignalObject()
{
}

bool ThreadBoolSignalObject::Wait()
{
  ThreadLock lock(*this);
  while (!condition)
  {
    if (!ThreadSignalObject::Wait()) return false;
  }
  condition = false;
  return true;
}

bool ThreadBoolSignalObject::Signal()
{
  ThreadLock lock(*this);
  condition = true;
  return ThreadSignalObject::Signal();
}

bool ThreadBoolSignalObject::Broadcast()
{
  ThreadLock lock(*this);
  condition = true;
  return ThreadSignalObject::Broadcast();
}

BBC_AUDIOTOOLBOX_END
