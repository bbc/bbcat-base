#ifndef __SIMPLE_THREAD_LOCK__
#define __SIMPLE_THREAD_LOCK__

#include "Thread.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A class to allow thread locking without the hassle of remembering to unlock the thread
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

protected:
  pthread_mutex_t mutex;
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
  ThreadLock(ThreadLockObject& lockobj) : obj(lockobj) {obj.Lock();}
  /*--------------------------------------------------------------------------------*/
  /** Const constructor to allow use in const methods
   */
  /*--------------------------------------------------------------------------------*/
  ThreadLock(const ThreadLockObject& lockobj) : obj(const_cast<ThreadLockObject&>(lockobj)) {obj.Lock();}
  /*--------------------------------------------------------------------------------*/
  /** Destructor unlocks ThreadLockObject
   */
  /*--------------------------------------------------------------------------------*/
  ~ThreadLock() {obj.Unlock();}

protected:
  ThreadLockObject& obj;
};

/*--------------------------------------------------------------------------------*/
/** Thread signalling base class - AVOID using as it doesn't handle initial conditions
 * Use ThreadBoolSignalObject for boolean conditions instead
 */
/*--------------------------------------------------------------------------------*/
class ThreadSignalObject : public ThreadLockObject
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
  pthread_cond_t cond;
};

/*--------------------------------------------------------------------------------*/
/** Improvement on the above which allows proper handling of the 'signalled before wait' situation
 */
/*--------------------------------------------------------------------------------*/
class ThreadBoolSignalObject : public ThreadSignalObject
{
public:
  ThreadBoolSignalObject(bool initial_condition = false);
  virtual ~ThreadBoolSignalObject();

  /*--------------------------------------------------------------------------------*/
  /** Wait for condition to be true
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
  volatile bool condition;
};

BBC_AUDIOTOOLBOX_END

#endif
