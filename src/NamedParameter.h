#ifndef __NAMED_PARAMETER__
#define __NAMED_PARAMETER__

#include "misc.h"
#include "3DPosition.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Interface class for NamedParameter templated classes
 */
/*--------------------------------------------------------------------------------*/
class INamedParameter : public JSONSerializable
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Assignment operator (requires upcasting using dynamic_cast<>)
   *
   * @note assigning a NamedParameter of a different type will result in no change
   */
  /*--------------------------------------------------------------------------------*/
  virtual INamedParameter& operator = (const INamedParameter& obj) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Comparison operator (requires upcasting using dynamic_cast<>)
   *
   * @note comparison with a NamedParameter of a different type will return false
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool operator == (const INamedParameter& obj) const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter back to its default
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Reset() = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return whether parameter has been set
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsSet() const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return textual name of parameter
   */
  /*--------------------------------------------------------------------------------*/
  virtual const char *GetName() const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return textual representation of parameter value
   */
  /*--------------------------------------------------------------------------------*/
  virtual std::string ToString() const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Convert string to value
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromString(const std::string& str) = 0;

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Assignment from JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual INamedParameter& operator = (const JSONValue& obj) = 0;
#endif
};

/*--------------------------------------------------------------------------------*/
/** Templated declaration of the above type
 *
 * @param TYPE type of parameter value (int, double, std::string, etc)
 *
 * @note MUST be derived from to supply name
 */
/*--------------------------------------------------------------------------------*/
template<typename TYPE>
class NamedParameter : public INamedParameter {
public:
  /*--------------------------------------------------------------------------------*/
  /** Default constructor - sets value to default value
   */
  /*--------------------------------------------------------------------------------*/
  NamedParameter() : INamedParameter(),
                     value(GetDefaultValue()),
                     valueset(false) {}
  /*--------------------------------------------------------------------------------*/
  /** Construction from value
   */
  /*--------------------------------------------------------------------------------*/
  NamedParameter(const TYPE&  obj) : INamedParameter(),
                                     value(GetDefaultValue()),
                                     valueset(false) {operator = (obj);}
#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Construction from JSON value
   */
  /*--------------------------------------------------------------------------------*/
  NamedParameter(const JSONValue&  obj) : INamedParameter(),
                                          value(GetDefaultValue()),
                                          valueset(false) {operator = (obj);}
#endif

  /*--------------------------------------------------------------------------------*/
  /** Copy constructor (upcasting as necessary)
   */
  /*--------------------------------------------------------------------------------*/
  NamedParameter(const INamedParameter& obj)  : INamedParameter(),
                                                value(GetDefaultValue()),
                                                valueset(false) {operator = (obj);}
  /*--------------------------------------------------------------------------------*/
  /** Destructor
   */
  /*--------------------------------------------------------------------------------*/
  virtual ~NamedParameter() {}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator (requires upcasting using dynamic_cast<>)
   *
   * @note assigning a NamedParameter of a different type will result in no change
   */
  /*--------------------------------------------------------------------------------*/
  virtual NamedParameter& operator = (const INamedParameter& obj)
  {
    const NamedParameter *p = dynamic_cast<const NamedParameter *>(&obj);
    if (p) *this = *p;
    return *this;
  }
  /*--------------------------------------------------------------------------------*/
  /** Concrete assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  virtual NamedParameter& operator = (const NamedParameter& obj) {value = obj.value; valueset = obj.valueset; return *this;}
  /*--------------------------------------------------------------------------------*/
  /** Assignment from value
   */
  /*--------------------------------------------------------------------------------*/
  virtual NamedParameter& operator = (const TYPE& obj) {value = obj; valueset = true; return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Cast to value type
   */
  /*--------------------------------------------------------------------------------*/
  virtual operator const TYPE& () const {return value;}

  /*--------------------------------------------------------------------------------*/
  /** Comparison operator (requires upcasting using dynamic_cast<>)
   *
   * @note comparison with a NamedParameter of a different type will return false
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool operator == (const INamedParameter& obj) const
  {
    const NamedParameter *p = dynamic_cast<const NamedParameter *>(&obj);
    return (p && (*p == *this));
  }

  /*--------------------------------------------------------------------------------*/
  /** Concrete type comparison
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool operator == (const NamedParameter<TYPE>& obj) const {return (value == obj.value);}
  virtual bool operator != (const NamedParameter<TYPE>& obj) const {return (value != obj.value);}
  
  /*--------------------------------------------------------------------------------*/
  /** Comparison with value
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool operator == (const TYPE& obj) const {return (value == obj);}
  virtual bool operator != (const TYPE& obj) const {return (value != obj);}

  /*--------------------------------------------------------------------------------*/
  /** Comparison between type and value
   */
  /*--------------------------------------------------------------------------------*/
  friend bool operator == (const TYPE& value, const NamedParameter<TYPE>& obj) {return obj.operator == (value);}
  friend bool operator != (const TYPE& value, const NamedParameter<TYPE>& obj) {return obj.operator != (value);}

  /*--------------------------------------------------------------------------------*/
  /** Get value
   */
  /*--------------------------------------------------------------------------------*/
  virtual const TYPE& Get()          const {return value;}
  virtual bool        Get(TYPE& obj) const {obj = value; return valueset;}

  /*--------------------------------------------------------------------------------*/
  /** Return writable reference to value (use with care!)
   *
   * @note if value is modified, MarkAsSet() MUST be called
   */
  /*--------------------------------------------------------------------------------*/
  virtual TYPE& GetWritable() {return value;}

  /*--------------------------------------------------------------------------------*/
  /** Set value and mark value as set
   */
  /*--------------------------------------------------------------------------------*/
  virtual TYPE& Set()                {valueset = true; return value;}
  virtual void  Set(const TYPE& obj) {value = obj; valueset = true;}

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter back to its default
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Reset() {value = GetDefaultValue(); valueset = false;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether parameter has been set
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsSet() const {return valueset;}

  /*--------------------------------------------------------------------------------*/
  /** Mark the parameter as set (if GetWritable() was used, for example)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void MarkAsSet() {valueset = true;}

  /*--------------------------------------------------------------------------------*/
  /** Return textual representation of parameter value
   */
  /*--------------------------------------------------------------------------------*/
  virtual std::string ToString() const {return StringFrom(value);}

  /*--------------------------------------------------------------------------------*/
  /** Convert string to value
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromString(const std::string& str) {bool success = Evaluate(str, value); valueset |= success; return success;}

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Assignment from JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual NamedParameter& operator = (const JSONValue& obj) {FromJSON(obj); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Return JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromJSON(const JSONValue& obj)
  {
    bool success = json::FromJSON(obj, value);
    valueset |= success;
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Cast to JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual operator JSONValue () const {return json::ToJSON(value);}

  /*--------------------------------------------------------------------------------*/
  /** Return JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ToJSON(JSONValue& obj) const {return json::ToJSON(value, obj);}
#endif

protected:
  /*--------------------------------------------------------------------------------*/
  /** Return default value (can be overridden)
   */
  /*--------------------------------------------------------------------------------*/
  virtual TYPE GetDefaultValue() const {return TYPE();}

protected:
  TYPE value;
  bool valueset;
};

#if ENABLE_JSON
namespace json
{
  /*--------------------------------------------------------------------------------*/
  /** Convert from list of NamedParameter objects to a JSON object
   *
   * @param list of NamedParameter objects
   * @param obj JSON object to be populated
   * @param all true to include parameters that are at their default
   */
  /*--------------------------------------------------------------------------------*/
  extern void ToJSON(const std::vector<INamedParameter *>& list, JSONValue& obj, bool all = false);
  extern void ToJSON(const INamedParameter * const *list, uint_t n, JSONValue& obj, bool all = false);
  /*--------------------------------------------------------------------------------*/
  /** Convert from a JSON object to a set of NamedParameters
   *
   * @param obj JSON object to be as the source
   * @param list of NamedParameter objects to extract
   * @param reset true to reset parameters that are not specified
   *
   * @return true if all [found] parameters were evaluated properly
   */
  /*--------------------------------------------------------------------------------*/
  extern bool FromJSON(const JSONValue& obj, const std::vector<INamedParameter *>& list, bool reset = true);
  extern bool FromJSON(const JSONValue& obj, INamedParameter * const *list, uint_t n, bool reset = true);
};
#endif

/*--------------------------------------------------------------------------------*/
/** Macro for named parameter with type and default default
 */
/*--------------------------------------------------------------------------------*/
#define NAMEDPARAMETER(type,name)                                                                       \
class __##name##Parameter : public NamedParameter<type>                                                 \
{                                                                                                       \
 public:                                                                                                \
  __##name##Parameter() : NamedParameter<type>() {}                                                     \
  __##name##Parameter(type obj) : NamedParameter<type>(obj) {}                                          \
  __##name##Parameter(const __##name##Parameter& obj) : NamedParameter<type>(obj) {}                    \
  __##name##Parameter& operator = (const type& val) {NamedParameter::operator = (val); return *this;}   \
  const char *GetName() const {return #name;}                                                           \
};                                                                                                      \
__##name##Parameter name;

/*--------------------------------------------------------------------------------*/
/** Macro for named hex parameter with type and default default
 */
/*--------------------------------------------------------------------------------*/
#define NAMEDHEXPARAMETER(type,name,fmt)                                                                                            \
class __##name##Parameter : public NamedParameter<type>                                                                             \
{                                                                                                                                   \
public:                                                                                                                             \
  __##name##Parameter() : NamedParameter<type>() {}                                                                                 \
  __##name##Parameter(type obj) : NamedParameter<type>(obj) {}                                                                      \
  __##name##Parameter(const __##name##Parameter& obj) : NamedParameter<type>(obj) {}                                                \
  __##name##Parameter& operator = (const type& val) {NamedParameter::operator = (val); return *this;}                               \
  const char  *GetName() const {return #name;}                                                                                      \
  std::string ToString() const {std::string res = StringFrom(value, fmt); return res;}                                              \
  bool        FromString(const std::string& str) {bool success = Evaluate(str, value, true); valueset |= success; return success;}  \
};                                                                                                                                  \
__##name##Parameter name;

/*--------------------------------------------------------------------------------*/
/** Macro for named time parameter with type and default default
 */
/*--------------------------------------------------------------------------------*/
#define NAMEDTIMEPARAMETER(name)                                                                                                \
class __##name##Parameter : public NamedParameter<uint64_t>                                                                     \
{                                                                                                                               \
public:                                                                                                                         \
  __##name##Parameter() : NamedParameter<uint64_t>() {}                                                                         \
  __##name##Parameter(uint64_t obj) : NamedParameter<uint64_t>(obj) {}                                                    \
  __##name##Parameter(const __##name##Parameter& obj) : NamedParameter<uint64_t>(obj) {}                                        \
  __##name##Parameter& operator = (const uint64_t& val) {NamedParameter::operator = (val); return *this;}                       \
  const char  *GetName() const {return #name;}                                                                                  \
  std::string ToString() const {std::string res = GenerateTime(value); return res;}                                             \
  bool        FromString(const std::string& str) {bool success = CalcTime(value, str); valueset |= success; return success;}    \
};                                                                                                                              \
__##name##Parameter name;

/*--------------------------------------------------------------------------------*/
/** Macro for named parameter with type, default default and specific format converting to text
 *
 * This macro is suggested for use with floating point parameters to control the precision when converted to text
 */
/*--------------------------------------------------------------------------------*/
#define NAMEDFORMATTEDPARAMETER(type,name,fmt)                                                          \
class __##name##Parameter : public NamedParameter<type>                                                 \
{                                                                                                       \
 public:                                                                                                \
  __##name##Parameter() : NamedParameter<type>() {}                                                     \
  __##name##Parameter(type obj) : NamedParameter<type>(obj) {}                                          \
  __##name##Parameter(const __##name##Parameter& obj) : NamedParameter<type>(obj) {}                    \
  __##name##Parameter& operator = (const type& val) {NamedParameter::operator = (val); return *this;}   \
  const char *GetName() const {return #name;}                                                           \
  std::string ToString() const {std::string res = FromString(value, fmt); return res;}                  \
};                                                                                                      \
__##name##Parameter name;

/*--------------------------------------------------------------------------------*/                    \
/** Macro for named parameter with type and specific default                                            \
 */                                                                                                     \
/*--------------------------------------------------------------------------------*/                    \
#define NAMEDPARAMETERDEF(type,name,def)                                                                \
class __##name##Parameter : public NamedParameter<type>                                                 \
{                                                                                                       \
public:                                                                                                 \
  __##name##Parameter() : NamedParameter<type>() {}                                                     \
  __##name##Parameter(type obj) : NamedParameter<type>(obj) {}                                          \
  __##name##Parameter(const __##name##Parameter& obj) : NamedParameter<type>(obj) {}                    \
  __##name##Parameter& operator = (const type& val) {NamedParameter::operator = (val); return *this;}   \
  const char *GetName() const {return #name;}                                                           \
protected:                                                                                              \
  virtual type GetDefaultValue() const {return def;}                                                    \
};                                                                                                      \
__##name##Parameter name;

BBC_AUDIOTOOLBOX_END

#endif
