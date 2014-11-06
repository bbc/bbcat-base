
#include <stdio.h>

#include <map>

#define DEBUG_LEVEL 0
#include "ParameterSet.h"

BBC_AUDIOTOOLBOX_START

const char *ParameterSet::DoubleFormatHuman = "%0.32le";
const char *ParameterSet::DoubleFormatExact = "#%016lx";

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

std::string ParameterSet::ToString() const
{
  std::map<std::string,std::string>::const_iterator it;
  std::string str;

  for (it = values.begin(); it != values.end(); ++it)
  {
    if (it != values.begin()) Printf(str, ", ");
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

ParameterSet& ParameterSet::Set(const std::string& name, bool val)
{
  std::string str;
  Printf(str, "%u", val ? 1 : 0);
  Set(name, str);

  return *this;
}

ParameterSet& ParameterSet::Set(const std::string& name, sint_t val)
{
  std::string str;
  Printf(str, "%d", val);
  Set(name, str);

  return *this;
}

ParameterSet& ParameterSet::Set(const std::string& name, slong_t val)
{
  std::string str;
  Printf(str, "%ld", val);
  Set(name, str);

  return *this;
}

ParameterSet& ParameterSet::Set(const std::string& name, double val, const char *fmt)
{
  std::string str;
  // this will FAIL to compile as 32-bit code
  // print double as hex encoded double
  Printf(str, fmt, val);
  Set(name, str);

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
  uint_t n;
  bool   success = false;

  if ((it != values.end()) && (sscanf(it->second.c_str(), "%u", &n) > 0))
  {
    val = (n != 0);
    success = true;
  }

  return success;
}

bool ParameterSet::Get(const std::string& name, sint_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && (sscanf(it->second.c_str(), "%d", &val) > 0));
}

bool ParameterSet::Get(const std::string& name, uint_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && (sscanf(it->second.c_str(), "%u", &val) > 0));
}

bool ParameterSet::Get(const std::string& name, slong_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && (sscanf(it->second.c_str(), "%ld", &val) > 0));
}

bool ParameterSet::Get(const std::string& name, ulong_t& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);
  return ((it != values.end()) && (sscanf(it->second.c_str(), "%lu", &val) > 0));
}

bool ParameterSet::Get(const std::string& name, double& val) const
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);

  if (it != values.end())
  {
    const char *str = it->second.c_str();

    // this will FAIL to compile as 32-bit code
    // values starting with a '#' are a 64-bit hex representation of the double value
    if ((str[0] == '#') && (sscanf(str + 1, "%lx", (ulong_t *)&val) > 0)) return true;

    // otherwise try to scan the string as a double
    if ((str[0] != '#') && (sscanf(str,     "%lf",            &val) > 0)) return true;
  }

  return false;
}


/*--------------------------------------------------------------------------------*/
/** Delete a parameter
 */
/*--------------------------------------------------------------------------------*/
bool ParameterSet::Delete(const std::string& name)
{
  const std::map<std::string,std::string>::const_iterator it = values.find(name);

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

BBC_AUDIOTOOLBOX_END
