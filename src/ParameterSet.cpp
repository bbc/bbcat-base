
#include <stdio.h>
#include <stdarg.h>

#include <map>

#define DEBUG_LEVEL 0
#include "ParameterSet.h"

BBC_AUDIOTOOLBOX_START

ParameterSet::ParameterSet(const ParameterSet& obj)
{
  operator = (obj);
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
ParameterSet& ParameterSet::operator = (const ParameterSet& obj)
{
  values = obj.values;

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

bool ParameterSet::Get(const std::string& name, bool& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && Evaluate(it->second.c_str(), val));
}

bool ParameterSet::Get(const std::string& name, sint_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && Evaluate(it->second, val));
}

bool ParameterSet::Get(const std::string& name, uint_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && Evaluate(it->second, val));
}

bool ParameterSet::Get(const std::string& name, slong_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && Evaluate(it->second, val));
}

bool ParameterSet::Get(const std::string& name, ulong_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && Evaluate(it->second, val));
}

bool ParameterSet::Get(const std::string& name, double& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && Evaluate(it->second, val));
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

bool ParameterSet::GetSubParameters(ParameterSet& parameters, const std::string& prefix) const
{
  std::string _prefix = prefix + ".";
  Iterator it;

  for (it = GetBegin(); it != GetEnd(); ++it)
  {
    // if the name starts with 'vbap.' create a corresponding parameters in vbapparameters
    if (it->first.find(_prefix) == 0) parameters.Set(it->first.substr(_prefix.length()), it->second);
  }

  return !parameters.IsEmpty();
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
    std::string key  = arg.substr(0, MIN(pos3, pos4));          // key
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

BBC_AUDIOTOOLBOX_END
