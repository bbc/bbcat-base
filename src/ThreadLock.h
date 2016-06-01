#ifndef __SIMPLE_THREAD_LOCK__
#define __SIMPLE_THREAD_LOCK__

#include "Thread.h"

#ifndef USE_PTHREADS
#include <mutex>
#include <condition_variable>
#endif

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A class to allow thread locking without the hassle of remembering to unlock the thread (pthreads version)
 *
 * Simple add a ThreadLockObject to your class and then whenever you want to lock a resource
 * within the class use a ThreadLock object.  On construction of the ThreadLock object the
 * ThreadLockObject will be locked and on destruction if the ThreadLock object the
 * ThreadLockObject will be unlocked
 */
/*--------------------------------------------------------------------------------*/
class ThreadLockObject
{
public:
  ThreadLockObject();
  virtual ~ThreadLockObject();

  /*--------------------------------------------------------------------------------*/
  /** Explicit lock of mutex (AVOID: use a ThreadLock object)
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Lock();

  /*--------------------------------------------------------------------------------*/
  /** Explicit unlock of mutex (AVOID: use a ThreadLock object)
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Unlock();

#ifndef USE_PTHREADS
protected:
  friend class ThreadLock;
#endif
  
protected:
#ifdef USE_PTHREADS
  pthread_mutex_t mutex;
#else
  std::recursive_mutex mutex;
  std::unique_lock<std::recursive_mutex> lock;
#endif
};

/*--------------------------------------------------------------------------------*/
/** Locking object - use this when you want to lock a ThreadLockObject
 */
/*--------------------------------------------------------------------------------*/
class ThreadLock
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Constructor locks ThreadLockObject
   */
  /*--------------------------------------------------------------------------------*/
  ThreadLock(ThreadLockObject& lockobj);
  /*--------------------------------------------------------------------------------*/
  /** Const constructor to allow use in const methods
   */
  /*--------------------------------------------------------------------------------*/
  ThreadLock(const ThreadLockObject& lockobj);
  /*--------------------------------------------------------------------------------*/
  /** Destructor unlocks ThreadLockObject
   */
  /*--------------------------------------------------------------------------------*/
  ~ThreadLock();

protected:
#ifdef USE_PTHREADS
  ThreadLockObject& obj;
#else
  std::lock_guard<std::recursive_mutex> guard;
#endif
};

/*--------------------------------------------------------------------------------*/
/** Thread signalling base class - AVOID using as it doesn't handle initial conditions
 * Use ThreadBoolSignalObject for boolean conditions instead
 */
/*--------------------------------------------------------------------------------*/
class ThreadSignalObject
#ifdef USE_PTHREADS
  : public ThreadLockObject
#endif
{
public:
  ThreadSignalObject();
  virtual ~ThreadSignalObject();

  /*--------------------------------------------------------------------------------*/
  /** Wait for condition to be triggered
   *
   * @note condition MUST NOT be set or this will wait forever!
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Wait();

  /*--------------------------------------------------------------------------------*/
  /** Signal first waiting thread
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Signal();

  /*--------------------------------------------------------------------------------*/
  /** Signal all waiting threads
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Broadcast();

protected:
  /*--------------------------------------------------------------------------------*/
  /** Set condition
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetReady() = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return whether condition has been met
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsReady() const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Reset ready state
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ClearReady() = 0;

protected:
#ifdef USE_PTHREADS
  pthread_cond_t          condition;
#else
  std::mutex              mutex;
  std::condition_variable condition;
#endif
};

/*--------------------------------------------------------------------------------*/
/** Boolean implementation which allows proper handling of the 'signalled before wait' situation
 */
/*--------------------------------------------------------------------------------*/
class ThreadBoolSignalObject : public ThreadSignalObject
{
public:
  ThreadBoolSignalObject(bool initial_condition = false);
  virtual ~ThreadBoolSignalObject();

protected:
  /*--------------------------------------------------------------------------------*/
  /** Set ready state
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetReady() {ready = true;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether condition has been met
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsReady() const {return ready;}

  /*--------------------------------------------------------------------------------*/
  /** Reset ready state
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ClearReady() {ready = false;}
  
protected:
  volatile bool ready;
};

BBC_AUDIOTOOLBOX_END

#endif
