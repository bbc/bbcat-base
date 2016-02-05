
#include <stdio.h>
#include <stdarg.h>

#include <map>

#define BBCDEBUG_LEVEL 0
#include "ParameterSet.h"

BBC_AUDIOTOOLBOX_START

ParameterSet::ParameterSet(const std::string& values)
{
  operator = (values);
}

ParameterSet::ParameterSet(const std::vector<std::string>& values)
{
  operator = (values);
}

ParameterSet::ParameterSet(const ParameterSet& obj)
{
  operator = (obj);
}

/*--------------------------------------------------------------------------------*/
/** Assignment operators
 */
/*--------------------------------------------------------------------------------*/
ParameterSet& ParameterSet::operator = (const std::string& values)
{
  std::vector<std::string> valueslist;

  // split lines into an list
  SplitString(values, valueslist, '\n');

  return operator = (valueslist);
}

ParameterSet& ParameterSet::operator = (const std::vector<std::string>& values)
{
  uint_t i;

  for (i = 0; i < values.size(); i++)
  {
    const std::string& str = values[i];
    size_t p;

    if ((p = str.find("=")) < std::string::npos)
    {
      // set as key=value
      Set(str.substr(0, p), str.substr(p + 1));
    }
  }

  return *this;
}

ParameterSet& ParameterSet::operator = (const ParameterSet& obj)
{
  // do not copy oneself
  if (&obj != this) values = obj.values;

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Returns whether this object contains all the keys and values of 'obj'
 */
/*--------------------------------------------------------------------------------*/
bool ParameterSet::Contains(const ParameterSet& obj) const
{
  std::map<std::string,std::string>::const_iterator it;

  for (it = obj.values.begin(); it != obj.values.end(); ++it)
  {
    std::string val;

    // if key doesn't exist or the values are different, return false
    if (!Get(it->first, val) || (val != it->second)) return false;
  }

  return true;
}

/*--------------------------------------------------------------------------------*/
/** Merge operator
 */
/*--------------------------------------------------------------------------------*/
ParameterSet& ParameterSet::operator += (const ParameterSet& obj)
{
  std::map<std::string,std::string>::const_iterator it;

  for (it = obj.values.begin(); it != obj.values.end(); ++it)
  {
    values[it->first] = it->second;
  }

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Removal operator
 *
 * Removes any parameters specified on obj that exist in this object
 */
/*--------------------------------------------------------------------------------*/
ParameterSet& ParameterSet::operator -= (const ParameterSet& obj)
{
  std::map<std::string,std::string>::const_iterator it;
  std::map<std::string,std::string>::iterator       it2;

  for (it = obj.values.begin(); it != obj.values.end(); ++it)
  {
    if ((it2 = values.find(it->first)) != values.end())
    {
      values.erase(it2);
    }
  }

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Return a string with each parameter and value (as string)
 */
/*--------------------------------------------------------------------------------*/
std::string ParameterSet::ToString(bool pretty) const
{
  std::map<std::string,std::string>::const_iterator it;
  std::string str;

  for (it = values.begin(); it != values.end(); ++it)
  {
    if (it != values.begin()) Printf(str, pretty ? "\n" : ", ");
    Printf(str, "%s %s", it->first.c_str(), it->second.c_str());
  }

  return str;
}

/*--------------------------------------------------------------------------------*/
/** Set parameter
 *
 * @note any existing value with the same name will be overwritten
 *
 */
/*--------------------------------------------------------------------------------*/
ParameterSet& ParameterSet::Set(const std::string& name, const std::string& val)
{
  values[name] = val;

  return *this;
}

ParameterSet& ParameterSet::Set(const std::string& name, const ParameterSet& val)
{
  std::string prefix = name + ".";
  Iterator it;

  // create a set of 'sub-parameters' - parameters with a prefix to indicate a sub object
  for (it = val.GetBegin(); it != val.GetEnd(); ++it)
  {
    Set(prefix + it->first, it->second);
  }

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Get value
 *
 * @param name parameter name
 * @param val reference to value to be set
 *
 * @return true if parameter found and value extracted
 */
/*--------------------------------------------------------------------------------*/
bool ParameterSet::Get(const std::string& name, std::string& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  if (it != values.end()) val = it->second;
  return (it != values.end());
}

/*--------------------------------------------------------------------------------*/
/** Delete a parameter
 */
/*--------------------------------------------------------------------------------*/
bool ParameterSet::Delete(const std::string& name)
{
  const std::map<std::string,std::string>::iterator it = values.find(name);

  if (it != values.end())
  {
    values.erase(it);
    return true;
  }

  return false;
}

/*--------------------------------------------------------------------------------*/
/** Return raw value (as stored) for given type or default if it does not exists
 */
/*--------------------------------------------------------------------------------*/
std::string ParameterSet::Raw(const std::string& name, const std::string& defval) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return (it != values.end()) ? it->second : defval; 
}

/*--------------------------------------------------------------------------------*/
/** Split full parameter name into prefix and suffix
 */
/*--------------------------------------------------------------------------------*/
bool ParameterSet::SplitSubParameter(const std::string& name, std::string& prefix, std::string& suffix)
{
  size_t p;

  if ((p = name.find(".")) < std::string::npos)
  {
    prefix = name.substr(0, p);
    suffix = name.substr(p + 1);
    return true;
  }

  return false;
}

/*--------------------------------------------------------------------------------*/
/** Return sub-parameters prefixed by 'prefix.'
 */
/*--------------------------------------------------------------------------------*/
bool ParameterSet::GetSubParameters(ParameterSet& parameters, const std::string& prefix) const
{
  std::string _prefix = prefix + ".";
  Iterator it;
  bool found = false;

  for (it = GetBegin(); it != GetEnd(); ++it)
  {
    // e.g. if the name starts with 'vbap.' create a corresponding parameters in parameters
    if (it->first.find(_prefix) == 0)
    {
      parameters.Set(it->first.substr(_prefix.length()), it->second);
      found = true;
    }
  }

  return found;
}

ParameterSet ParameterSet::GetSubParameters(const std::string& prefix) const
{
  ParameterSet parameters;
  GetSubParameters(parameters, prefix);
  return parameters;
}

bool ParameterSet::Get(const std::string& name, ParameterSet& val) const
{
  // extract sub-parameters using name as prefix
  return GetSubParameters(val, name);
}

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
std::string ParameterSet::GenerateMessage(const std::string& format, bool allowempty) const
{
  std::string msg = format;
  size_t pos1 = 0, pos2;

  while (((pos1 = msg.find("{", pos1)) < std::string::npos) &&
         ((pos2 = msg.find("}", pos1)) < std::string::npos))
  {
    std::string str1 = msg.substr(0, pos1);                     // text before {
    std::string str2 = msg.substr(pos2 + 1);                    // text after }
    std::string arg  = msg.substr(pos1 + 1, pos2 - pos1 - 1);   // text between { and }
    size_t      pos3 = arg.find(":");                           // key:fmt split
    size_t      pos4 = arg.find("?");                           // key?a:b split
    std::string key  = arg.substr(0, std::min(pos3, pos4));     // key
    std::string val;                                            // string that will end up with printable value in it

    if (allowempty || Exists(key))                              // make sure key exists
    {
      if (pos3 < std::string::npos)                             // format string has been specified
      {
        if (pos4 < pos3)
        {
          // tertiary operator found
          bool bval;

          // get boolean value
          if (Get(key, bval))
          {
            // choose a or b
            val = bval ? arg.substr(pos4 + 1, pos3 - pos4 - 1) : arg.substr(pos3 + 1);
          }
          else Get(key, val);
        }
        else
        {
          std::string fmt = arg.substr(pos3 + 1);               // format string
          std::string str;
          double  offset = 0.0;
          double  fval;
          ulong_t luval;
          slong_t lsval;
          uint_t  uval;
          sint_t  sval;

          // assume this is an offset
          if ((fmt[0] == '+') || (fmt[0] == '-'))
          {
            size_t pos;

            Evaluate(fmt, offset);

            if ((pos = fmt.find(":")) < std::string::npos)
            {
              fmt = fmt.substr(pos + 1);                        // find actual format string
            }
            else fmt = "";
          }

          // attempt to extract the value in the correct format for the format string
          if ((fmt.find("lu") < std::string::npos) || (fmt.find("lx") < std::string::npos))
          {
            if (Get(key, luval)) Printf(val, fmt.c_str(), luval + (slong_t)offset);
          }
          else if ((fmt.find("u") < std::string::npos) || (fmt.find("x") < std::string::npos))
          {
            if (Get(key, uval)) Printf(val, fmt.c_str(), uval + (sint_t)offset);
          }
          else if (fmt.find("ld") < std::string::npos)
          {
            if (Get(key, lsval)) Printf(val, fmt.c_str(), lsval + (slong_t)offset);
          }
          else if (fmt.find("d") < std::string::npos)
          {
            if (Get(key, sval)) Printf(val, fmt.c_str(), sval + (sint_t)offset);
          }
          else if (fmt.find("f") < std::string::npos)
          {
            if (Get(key, fval)) Printf(val, fmt.c_str(), fval + offset);
          }
          else if (fmt.find("s") < std::string::npos)
          {
            if (Get(key, str)) Printf(val, fmt.c_str(), str.c_str());
          }
          else Get(key, val);
        }
      }
      else Get(key, val);
    }
    // if key does not exist, leave it as it is (including braces)
    else val = msg.substr(pos1, pos2 + 1 - pos1);

    // re-construct new string
    msg  = str1 + val + str2;
    
    // generate new starting point for search
    pos1 = str1.length() + val.length();
  }
  
  return msg;
}

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
bool ParameterSet::FindCombination(const std::vector<std::string>& strings, std::string& res) const
{
  uint_t i;
  bool   found = false;

  for (i = 0; !found && (i < strings.size()); i++)
  {
    ParameterSet subvars;

    if (Get(strings[i], res))
    {
      BBCDEBUG2(("Found '%s' in {%s}: %s", strings[i].c_str(), ToString().c_str(), res.c_str())); 
      found = true;
    }
    else if (Get(strings[i], subvars))
    {
      found = subvars.FindCombination(strings, res);
    }
  }

  if (!found) found = Get("default", res);

  return found;
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Return object as JSON object
 */
/*--------------------------------------------------------------------------------*/
json_spirit::mObject ParameterSet::ToJSON() const
{
  json_spirit::mObject obj;
  ToJSON(obj);
  return obj;
}

void ParameterSet::ToJSON(json_spirit::mObject& obj) const
{
  Iterator it;
    
  for (it = GetBegin(); it != GetEnd(); ++it)
  {
    const std::string& name = it->first;
    size_t pos;

    // detect sub-objects using '.' in name
    if ((pos = name.find(".")) != std::string::npos)
    {
      std::string subname = name.substr(0, pos);

      // only process sub-object for FIRST entry!
      if (obj.find(name) == obj.end())
      {
        ParameterSet subset;
        if (Get(subname, subset)) obj[subname] = subset.ToJSON();
      }
    }
    else obj[name] = it->second;
  }
}

bool ParameterSet::Get(const std::string& name, json_spirit::mObject& obj) const
{
  ParameterSet subset;
  bool success = false;

  if (Get(name, subset))
  {
    obj[name] = subset.ToJSON();
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set object from JSON
 */
/*--------------------------------------------------------------------------------*/
void ParameterSet::FromJSON(const json_spirit::mObject& obj)
{
  json_spirit::mObject::const_iterator it;

  for (it = obj.begin(); it != obj.end(); ++it)
  {
    Set(it->first, it->second);
  }
}

ParameterSet& ParameterSet::Set(const std::string& name, const json_spirit::mValue& value)
{
  switch (value.type())
  {
    case json_spirit::obj_type:
      Set(name, value.get_obj());
      break;

    case json_spirit::array_type:
    {
      const json_spirit::mArray& array = value.get_array();
      json_spirit::mArray::const_iterator it;
      uint_t i;

      for (i = 0, it = array.begin(); it != array.end(); ++it, ++i)
      {
        std::string name1;
        Printf(name1, "%s.%u", name.c_str(), i);
        Set(name1, *it);
      }
      break;
    }

    default:
      Set(name, value.get_str());
      break;
  }

  return *this;
}

ParameterSet& ParameterSet::Set(const std::string& name, const json_spirit::mObject& obj)
{
  ParameterSet subset;
  subset.FromJSON(obj);
  Set(name, subset);
  return *this;
}

bool FromJSON(const json_spirit::mValue& _val, ParameterSet& val)
{
  bool success = (_val.type() == json_spirit::obj_type);

  if (success)
  {
    val.FromJSON(_val.get_obj());
  }
  
  return success;
}

json_spirit::mValue ToJSON(const ParameterSet& val)
{
  return val.ToJSON();
}
#endif

BBC_AUDIOTOOLBOX_END
