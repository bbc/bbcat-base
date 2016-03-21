#ifndef __SYSTEM_PARAMETERS__
#define __SYSTEM_PARAMETERS__

#include "ParameterSet.h"
#include "ThreadLock.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Global registry of system parameters
 */
/*--------------------------------------------------------------------------------*/
class SystemParameters
{
public:
  virtual ~SystemParameters() {}
  
  /*--------------------------------------------------------------------------------*/
  /** Get single instance of system parameters object
   */
  /*--------------------------------------------------------------------------------*/
  static SystemParameters& Get()
  {
    static SystemParameters parameters;
    return parameters;
  }

  /*--------------------------------------------------------------------------------*/
  /** Get value
   *
   * Works like ParameterSet::Get()
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  bool Get(const std::string& name, T& val) const
  {
    ThreadLock lock(tlock);
    return parameters.Get(name, val);
  }

  /*--------------------------------------------------------------------------------*/
  /** Set value
   *
   * Works like ParameterSet::Set()
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  SystemParameters& Set(const std::string& name, const T& val)
  {
    ThreadLock lock(tlock);
    parameters.Set(name, val);
    return *this;
  }

  /*--------------------------------------------------------------------------------*/
  /** Return whether a parameter exists
   */
  /*--------------------------------------------------------------------------------*/
  bool Exists(const std::string& name) const
  {
    ThreadLock lock(tlock);
    return parameters.Exists(name);
  }

protected:
  SystemParameters() {}
  
protected:
  ThreadLockObject tlock;
  ParameterSet     parameters;
};
  
BBC_AUDIOTOOLBOX_END

#endif
