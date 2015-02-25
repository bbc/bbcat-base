#ifndef __PARAMETER_SET__
#define __PARAMETER_SET__

#include <string>
#include <map>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Collection of parameters, each with name/value pair with type conversion
 *
 */
/*--------------------------------------------------------------------------------*/
class ParameterSet
{
public:
  ParameterSet() {}
  ParameterSet(const ParameterSet& obj);
  ~ParameterSet() {}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  ParameterSet& operator = (const ParameterSet& obj);

  /*--------------------------------------------------------------------------------*/
  /** Comparison operators
   */
  /*--------------------------------------------------------------------------------*/
  bool operator == (const ParameterSet& obj) const {return map_compare(values, obj.values);}
  bool operator != (const ParameterSet& obj) const {return !operator == (obj);}

  /*--------------------------------------------------------------------------------*/
  /** Returns whether this object contains all the keys and values of 'obj'
   */
  /*--------------------------------------------------------------------------------*/
  bool Contains(const ParameterSet& obj) const;

  /*--------------------------------------------------------------------------------*/
  /** Merge operator
   */
  /*--------------------------------------------------------------------------------*/
  ParameterSet& operator += (const ParameterSet& obj);
  friend ParameterSet operator + (const ParameterSet& obj1, const ParameterSet& obj2) {ParameterSet res = obj1; res += obj2; return res;}
  
  /*--------------------------------------------------------------------------------*/
  /** Removal operator
   *
   * Removes any parameters specified on obj that exist in this object
   */
  /*--------------------------------------------------------------------------------*/
  ParameterSet& operator -= (const ParameterSet& obj);
  friend ParameterSet operator - (const ParameterSet& obj1, const ParameterSet& obj2) {ParameterSet res = obj1; res -= obj2; return res;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether parameter set is empty
   */
  /*--------------------------------------------------------------------------------*/
  bool IsEmpty() const {return values.empty();}

  /*--------------------------------------------------------------------------------*/
  /** Clear parameter set
   */
  /*--------------------------------------------------------------------------------*/
  void Clear() {values.clear();}

  /*--------------------------------------------------------------------------------*/
  /** Return a string with each parameter and value (as string)
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToString(bool pretty = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Set parameter
   *
   * @note any existing value with the same name will be overwritten
   * @note they return a reference to the object to allow chaining
   */
  /*--------------------------------------------------------------------------------*/
  ParameterSet& Set(const std::string& name, const std::string&  val);
  ParameterSet& Set(const std::string& name, const char         *val) {return Set(name, std::string(val));}
  ParameterSet& Set(const std::string& name, bool                val) {return Set(name, StringFrom(val));}
  ParameterSet& Set(const std::string& name, uint_t              val, const char *fmt = "%u") {return Set(name, StringFrom(val, fmt));}
  ParameterSet& Set(const std::string& name, sint_t              val, const char *fmt = "%d") {return Set(name, StringFrom(val, fmt));}
  ParameterSet& Set(const std::string& name, slong_t             val, const char *fmt = "%ld") {return Set(name, StringFrom(val, fmt));}
  ParameterSet& Set(const std::string& name, double              val, const char *fmt = DoubleFormatHuman) {return Set(name, StringFrom(val, fmt));}
  ParameterSet& Set(const std::string& name, const ParameterSet& val);

  /*--------------------------------------------------------------------------------*/
  /** Return whether a parameter exists
   */
  /*--------------------------------------------------------------------------------*/
  bool Exists(const std::string& name) const {return (values.find(name) != values.end());}

  /*--------------------------------------------------------------------------------*/
  /** Get start and iterators to allow iteration through all values
   */
  /*--------------------------------------------------------------------------------*/
  typedef std::map<std::string,std::string>::const_iterator Iterator;
    
  Iterator GetBegin() const {return values.begin();}
  Iterator GetEnd()   const {return values.end();}

  /*--------------------------------------------------------------------------------*/
  /** Get value
   *
   * @param name parameter name
   * @param val reference to value to be set
   *
   * @return true if parameter found and value extracted
   */
  /*--------------------------------------------------------------------------------*/
  bool Get(const std::string& name, std::string&  val) const;
  bool Get(const std::string& name, bool&         val) const;
  bool Get(const std::string& name, sint_t&       val) const;
  bool Get(const std::string& name, uint_t&       val) const;
  bool Get(const std::string& name, slong_t&      val) const;
  bool Get(const std::string& name, ulong_t&      val) const;
  bool Get(const std::string& name, double&       val) const;
  bool Get(const std::string& name, ParameterSet& val) const;

  /*--------------------------------------------------------------------------------*/
  /** Delete a parameter
   */
  /*--------------------------------------------------------------------------------*/
  bool Delete(const std::string& name);

  /*--------------------------------------------------------------------------------*/
  /** Return raw value (as stored) for given type or default if it does not exists
   */
  /*--------------------------------------------------------------------------------*/
  std::string Raw(const std::string& name, const std::string& defval = "") const;

  /*--------------------------------------------------------------------------------*/
  /** Return sub-parameters prefixed by 'prefix.'
   */
  /*--------------------------------------------------------------------------------*/
  bool         GetSubParameters(ParameterSet& parameters, const std::string& prefix) const;
  ParameterSet GetSubParameters(const std::string& prefix) const;

  /*--------------------------------------------------------------------------------*/
  /** Create a message using the supplied format
   *
   * @param format format string (see below)
   * @param allowempty true to replace non-existent keys with nothing (otherwise non-existing keys are left as is)
   *
   * @return message
   *
   * The format string uses '{' and '}' to specify keys and parameter format:
   *   {<key>[:<offset>][:[<fmt>]]} is replaced by the value of key <key>, optionally interpreted and formatted according to <fmt>s
   *   <offset> can be '+<n>' or '-<n>', can be floating point (*requires* <fmt> specification)
   *   <fmt> can be:
   *     %lf - double
   *     %d  - signed integer
   *     %ld - signed long integer
   *     %u  - unsigned integer
   *     %lu - unsigned long integer
   *     %x  - unsigned integer in hex
   *     %lx - unsigned long integer in hex
   *     %s  - string
   *     Field size and signficant figures are also supported as in printf
   *   If no <fmt> is specified, the value is treated as a string
   *  
   *   {<key>?<true-string>:<false-string>} is replaced by the value of key <key> used as a boolean to choose either <true-string> or <false-string> (tertiary operator)
   *
   * Examples:
   *   {objectid}
   *   {value:%0.3lf}
   *   {enabled?Enabled:Disabled}
   *   {name:%-30s}
   *   {id:%016lx}
   *   {objectindex:+1:%u}
   */
  /*--------------------------------------------------------------------------------*/
  std::string GenerateMessage(const std::string& format, bool allowempty = true) const;

protected:
  std::map<std::string,std::string> values;
};

BBC_AUDIOTOOLBOX_END

#endif
