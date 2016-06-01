#ifndef __SYSTEM_PARAMETERS__
#define __SYSTEM_PARAMETERS__

#include "ParameterSet.h"
#include "ThreadLock.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Global registry of system parameters
 *
 * This object can be used to store information to be used throughout the system
 * such as paths and parameters,
 *
 * During construction (via singleton access), it determines the location of the
 * 'share' folder where each library stores its shared data and the current user's
 * home directory, see 'Predefined Values', below
 *
 * Paths (including the 'share' directory described above) should be relative to other
 * entries, where possible.  For example, the 'share' folder defaults to:
 * '{installdir}/share' and the '{installdir}' is replaced by the value of 'installdir'
 * when access via GetSubstituted() (NOTE: Get() will return the true, unsubstituted
 * value!).  This mechanism should be used for paths but *can* be used by anything.
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
  static SystemParameters& Get();

  /*--------------------------------------------------------------------------------*/
  /** Get string value and substitute other referenced variables
   *
   * @note the return is passed through Substitute()
   */
  /*--------------------------------------------------------------------------------*/
  bool GetSubstituted(const std::string& name, std::string& val) const;

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
  bool Exists(const std::string& name) const;

  /*--------------------------------------------------------------------------------*/
  /** Replace {} entries from other values
   *
   * This will look for other values surrounded by '{' and '}' and replace them with
   * the values' value.  '{env:<envvar>}' means environment var '<envvar>' UNLESS
   * a parameter exists with the name 'env:<envvar>'.
   *
   * @param str string to replace entries in
   * @param replaceunknown true to replace unknown entries by blank, false to leave as is
   *
   * @return string with substitutions
   */
  /*--------------------------------------------------------------------------------*/
  std::string Substitute(const std::string& str, bool replaceunknown = true) const;

  /*--------------------------------------------------------------------------------*/
  /** Predefined Values
   */
  /*--------------------------------------------------------------------------------*/
  static const std::string installdirkey;   ///< base installation directory (e.g. /usr/local for Mac)
  static const std::string sharedirkey;     ///< shared data directory
  static const std::string homedirkey;      ///< current user's home directory
  
protected:
  SystemParameters();

  /*--------------------------------------------------------------------------------*/
  /** Read values from .conf style file
   */
  /*--------------------------------------------------------------------------------*/
  bool ReadFromFile(const std::string& filename);
  
protected:
  ThreadLockObject tlock;
  ParameterSet     parameters;
};
  
BBC_AUDIOTOOLBOX_END

#endif
