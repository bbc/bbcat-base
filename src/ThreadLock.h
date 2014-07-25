#ifndef __SIMPLE_THREAD_LOCK__
#define __SIMPLE_THREAD_LOCK__

#include <pthread.h>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

class ThreadLockObject {
public:
  ThreadLockObject();
  virtual ~ThreadLockObject();

  virtual bool Lock();
  virtual bool Unlock();

  void PulseLock()    {Lock(); Unlock();}
  void PulseRelease() {Unlock(); Lock();}

protected:
  pthread_mutex_t mutex;
};

class ThreadSignalObject : public ThreadLockObject {
public:
  ThreadSignalObject();
  virtual ~ThreadSignalObject();

  virtual bool Wait();
  virtual bool Signal();
  virtual bool Broadcast();

protected:
  pthread_cond_t cond;
};

class ThreadBoolSignalObject : public ThreadSignalObject {
public:
  ThreadBoolSignalObject(bool initial_condition = false);
  virtual ~ThreadBoolSignalObject();

  virtual bool Wait();
  virtual bool Signal();
  virtual bool Broadcast();
    
protected:
  volatile bool condition;
};

class ThreadLock {
public:
  ThreadLock(ThreadLockObject& lockobj) : obj(lockobj) {obj.Lock();}
  ~ThreadLock() {obj.Unlock();}

protected:
  ThreadLockObject& obj;
};

BBC_AUDIOTOOLBOX_END

#endif
