#ifndef __NAMED_PARAMETER__
#define __NAMED_PARAMETER__

#if ENABLE_JSON
#include <json_spirit/json_spirit.h>
#endif

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Interface class for NamedParameter templated classes
 */
/*--------------------------------------------------------------------------------*/
class INamedParameter
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

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Assignment from JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual INamedParameter& operator = (const json_spirit::mValue& _value) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromJSON(const json_spirit::mValue& _value) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual json_spirit::mValue ToJSON() const = 0;
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
  NamedParameter(const TYPE&  _value) : INamedParameter(),
                                        value(GetDefaultValue()),
                                        valueset(false) {operator = (_value);}
#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Construction from JSON value
   */
  /*--------------------------------------------------------------------------------*/
  NamedParameter(const json_spirit::mValue&  _value) : INamedParameter(),
                                                       value(GetDefaultValue()),
                                                       valueset(false) {operator = (_value);}
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
  virtual NamedParameter& operator = (const TYPE& _value) {value = _value; valueset = true; return *this;}

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
  virtual bool operator == (const NamedParameter& obj) const {return (value == obj.value);}

  /*--------------------------------------------------------------------------------*/
  /** Comparison with value
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool operator == (const TYPE& _value) const {return (value == _value);}

  /*--------------------------------------------------------------------------------*/
  /** Get value
   */
  /*--------------------------------------------------------------------------------*/
  virtual const TYPE& Get()             const {return value;}
  virtual bool        Get(TYPE& _value) const {_value = value; return valueset;}

  /*--------------------------------------------------------------------------------*/
  /** Set value
   */
  /*--------------------------------------------------------------------------------*/
  virtual TYPE& Set()                   {valueset = true; return value;}
  virtual void  Set(const TYPE& _value) {value = _value; valueset = true;}

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
  /** Return textual representation of parameter value
   */
  /*--------------------------------------------------------------------------------*/
  virtual std::string ToString() const {return StringFrom(value);}

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Assignment from JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual NamedParameter& operator = (const json_spirit::mValue& _value) {FromJSON(_value); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Return JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromJSON(const json_spirit::mValue& _value)
  {
    bool success = bbcat::FromJSON(_value, value);
    valueset |= success;
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Cast to JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual operator json_spirit::mValue () const {return bbcat::ToJSON(value);}

  /*--------------------------------------------------------------------------------*/
  /** Return JSON value
   */
  /*--------------------------------------------------------------------------------*/
  virtual json_spirit::mValue ToJSON() const {return bbcat::ToJSON(value);}
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
/*--------------------------------------------------------------------------------*/
/** Convert from list of NamedParameter objects to a JSON object
 *
 * @param list of NamedParameter objects
 * @param obj JSON object to be populated
 * @param all true to include parameters that are at their default
 */
/*--------------------------------------------------------------------------------*/
extern void ToJSON(const std::vector<INamedParameter *>& list, json_spirit::mObject& obj, bool all = false);
extern void ToJSON(const INamedParameter * const *list, uint_t n, json_spirit::mObject& obj, bool all = false);
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
extern bool FromJSON(const json_spirit::mObject& obj, const std::vector<INamedParameter *>& list, bool reset = true);
extern bool FromJSON(const json_spirit::mObject& obj, INamedParameter * const *list, uint_t n, bool reset = true);
#endif

/*--------------------------------------------------------------------------------*/
/** Macro for named parameter with type and default default
 */
/*--------------------------------------------------------------------------------*/
#define NAMEDPARAMETER(type,name)                                       \
class __##name##Parameter : public NamedParameter<type>                 \
{                                                                       \
public:                                                                 \
  virtual __##name##Parameter& operator = (const type& val) {NamedParameter::operator = (val); return *this;} \
  virtual const char *GetName() const {return #name;}                   \
};                                                                      \
__##name##Parameter name;

/*--------------------------------------------------------------------------------*/
/** Macro for named parameter with type and specific default
 */
/*--------------------------------------------------------------------------------*/
#define NAMEDPARAMETERDEF(type,name,def)                                \
class __##name##Parameter : public NamedParameter<type>                 \
{                                                                       \
public:                                                                 \
  virtual __##name##Parameter& operator = (const type& val) {NamedParameter::operator = (val); return *this;} \
  virtual const char *GetName() const {return #name;}                   \
protected:                                                              \
  virtual type GetDefaultValue() const {return def;}                    \
};                                                                      \
__##name##Parameter name;

BBC_AUDIOTOOLBOX_END

#endif
