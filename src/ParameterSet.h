#ifndef __PARAMETER_SET__
#define __PARAMETER_SET__

#include <string>
#include <map>
#include <vector>

#if ENABLE_JSON
#include <json_spirit/json_spirit.h>
#endif

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
  ParameterSet(const std::string& values);                      // lines of key=value strings
  ParameterSet(const std::vector<std::string>& values);         // array of key=value strings
  ParameterSet(const ParameterSet& obj);
#if ENABLE_JSON
  ParameterSet(const json_spirit::mObject& obj) {FromJSON(obj);}
#endif
  ~ParameterSet() {}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operators
   */
  /*--------------------------------------------------------------------------------*/
  ParameterSet& operator = (const std::string& values);                      // lines of key=value strings
  ParameterSet& operator = (const std::vector<std::string>& values);         // array of key=value strings
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
  ParameterSet& Set(const std::string& name, ulong_t             val, const char *fmt = "%lu") {return Set(name, StringFrom(val, fmt));}
  ParameterSet& Set(const std::string& name, slong_t             val, const char *fmt = "%ld") {return Set(name, StringFrom(val, fmt));}
  ParameterSet& Set(const std::string& name, ullong_t            val, const char *fmt = "%llu") {return Set(name, StringFrom(val, fmt));}
  ParameterSet& Set(const std::string& name, sllong_t            val, const char *fmt = "%lld") {return Set(name, StringFrom(val, fmt));}
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
  bool Get(const std::string& name, sllong_t&     val) const;
  bool Get(const std::string& name, ullong_t&     val) const;
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

  /*--------------------------------------------------------------------------------*/
  /** Find combinations of string array in hierarchical parameter sets 
   *
   * @param strings a list of strings to search for
   * @param res the resultant string
   *
   * @return true if combination found
   *
   * @note this function is recursive with a maximum depth governed by the depth of the ParameterSet
   *
   * It aims to find a combination of strings[] in the current parameter set and return the value using
   * the following strategy:
   *
   * For each string in strings[]:
   *   if that key exists and is not a stub for sub-values then return the value for that key
   *   if that key exists and is a stub for sub-values then start the search again on the sub-values
   *   if no results found at each level but 'default' key exists return the value for that key 
   *
   * e.g. for the ParameterSet:
   *
   *   values.a       red
   *   values.a.b     green
   *   values.a.b.c   blue
   *   values.a.c.b   pink
   *   values.a.c     yellow
   *   values.default black
   *
   * Would give the following for the specified arrays:
   *
   *   ['a']         : 'red'
   *   ['b']         : 'black' (because of 'default')
   *   ['a','b']     : 'green' ('a' then 'b')
   *   ['a','c']     : 'yellow' ('a' then 'c')
   *   ['a','b','c'] : 'blue' ('a' then 'b' then 'c')
   *   ['a','c','b'] : 'pink' ('a' then 'c' then 'b')
   */
  /*--------------------------------------------------------------------------------*/
  bool FindCombination(const std::vector<std::string>& strings, std::string& res) const;

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Return object as JSON object
   */
  /*--------------------------------------------------------------------------------*/
  json_spirit::mObject ToJSON() const;
  void ToJSON(json_spirit::mObject& obj) const;
  bool Get(const std::string& name, json_spirit::mObject& obj) const;
  operator json_spirit::mObject () const {return ToJSON();}

  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON string
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToJSONString() const {return json_spirit::write(ToJSON(), json_spirit::pretty_print);}

  /*--------------------------------------------------------------------------------*/
  /** Set object from JSON
   */
  /*--------------------------------------------------------------------------------*/
  void FromJSON(const json_spirit::mObject& obj);
  ParameterSet& Set(const std::string& name, const json_spirit::mObject& obj);
  ParameterSet& Set(const std::string& name, const json_spirit::mValue& value);
  ParameterSet& operator = (const json_spirit::mObject& obj) {FromJSON(obj); return *this;}
#endif

protected:
  std::map<std::string,std::string> values;
};

#if ENABLE_JSON
extern bool                FromJSON(const json_spirit::mValue& _val, ParameterSet& val);
extern json_spirit::mValue ToJSON(const ParameterSet& val);
#endif

BBC_AUDIOTOOLBOX_END

#endif
