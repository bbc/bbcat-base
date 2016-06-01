#ifndef __BBCAT_JSON__
#define __BBCAT_JSON__

#include "misc.h"

#if ENABLE_JSON
#include <json/json.h>
#endif

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** JSON support
 *
 * Below are a bunch of functions to make JSON handling easier, particularly
 * converting to and from basic types
 *
 * However, ANY type (basic, complex or classes) can be converted to and from JSON
 * by supporting ToJSON() and FromJSON member functions (optionally deriving from
 * JSONSerializable)
 *
 * Ultimately, for any type for which JSON support is desired (but which is *not*
 * derived from JSONSerializable), there should be two global functions:
 *
 * bool FromJSON(const JSONValue& obj, <type>& val);
 * void ToJSON(const <type>& val, JSONValue& obj);
 * 
 * This approach allows for JSON support of things that cannot be modified (e.g.
 * basic types or external classes)
 *
 * If JSON support is NOT available, the JSONSerializable interface is left as an
 * empty interface
 *
 * All the functions are defined within the 'json' namespace to isolate them from
 * the rest of the code and make it easier to directly reference the namspace'd
 * versions (to avoid confusion with internal or member functions)
 *
 * Raw JSON interface uses jsoncpp:
 * https://github.com/open-source-parsers/jsoncpp
 *
 * Usage
 *
 * json::FromJSON(<jsonobj>, <obj>)    - Convert from a JSON object (JSONValue) to any supported object
 * json::ToJSON(<obj>, <jsonobj>)      - Convert from any supported object to a JSON object (JSONValue)
 *
 * json::FromJSON(<jsonobj>, <member>, <obj>) - Convert from member <member> of a JSON object (JSONValue) to any supported object
 * json::FromJSON(<jsonobj>, <index>, <obj>)  - Convert from index  <index> of a JSON array (JSONValue) to any supported object
 *
 * json::FromJSONString(<str>, <obj>)  - Convert from a JSON string to any supported object
 * json::ToJSONString(<obj>, <pretty>) - Returns a [pretty] JSON string from any supported object
 *
 * Example:
 *
 *  json::ToJSON(GetID(),           obj["id"]);
 *  json::ToJSON(GetName(),         obj["name"]);
 *  json::ToJSON(GetStartTime(),    obj["starttime"]);
 *  json::ToJSON(GetDuration(),     obj["duration"]);
 *  json::ToJSON(GetStartChannel(), obj["startchannel"]);
 *  json::ToJSON(GetChannelCount(), obj["channelcount"]);
 *
 * NOTE: although assignment operators exist in the jsoncpp library (i.e. assigning
 *       directly to a JSONValue) this doesn't support complex types and also different
 *       versions of the libraries have differing levels of support for this.  The
 *       approach described above works for everything (that's supported)
 */
/*--------------------------------------------------------------------------------*/

#if ENABLE_JSON

// old versions of jsoncpp don't have name() member of iterator class and use memberName() instead
#ifdef OLD_JSON_CPP
#define JSON_MEMBER_NAME(it) std::string((it).memberName())
#else
#define JSON_MEMBER_NAME(it) ((it).name())
#endif

/*--------------------------------------------------------------------------------*/
/** typedef to make use of JSON stuff easier
 */
/*--------------------------------------------------------------------------------*/
typedef Json::Value JSONValue;

namespace json
{
  /*--------------------------------------------------------------------------------*/
  /** Conversions from JSON to basic types
   */
  /*--------------------------------------------------------------------------------*/
  extern bool FromJSON(const JSONValue& obj, bool& val);
  extern bool FromJSON(const JSONValue& obj, sint_t& val);
  extern bool FromJSON(const JSONValue& obj, uint_t& val);
  extern bool FromJSON(const JSONValue& obj, sint64_t& val);
  extern bool FromJSON(const JSONValue& obj, uint64_t& val);
  extern bool FromJSON(const JSONValue& obj, float& val);
  extern bool FromJSON(const JSONValue& obj, double& val);
  extern bool FromJSON(const JSONValue& obj, std::string& val);
  // for completeness
  extern bool FromJSON(const JSONValue& obj, JSONValue& val);

  /*--------------------------------------------------------------------------------*/
  /** Templated access to member of JSON object
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  bool FromJSON(const JSONValue& _val, const std::string& member, T& val)
  {
    return (_val.isObject() && _val.isMember(member) && FromJSON(_val[member], val));
  }

  /*--------------------------------------------------------------------------------*/
  /** Templated access to array index of JSON object
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  bool FromJSON(const JSONValue& _val, uint_t index, T& val)
  {
    return (_val.isArray() && (index < (uint_t)_val.size()) && FromJSON(_val[index], val));
  }

  /*--------------------------------------------------------------------------------*/
  /** Conversion from [possibly complex] types to JSON value
   */
  /*--------------------------------------------------------------------------------*/
  extern void ToJSON(bool val, JSONValue& obj);
  extern void ToJSON(uint_t val, JSONValue& obj);
  extern void ToJSON(sint_t val, JSONValue& obj);
  extern void ToJSON(uint64_t val, JSONValue& obj);
  extern void ToJSON(sint64_t val, JSONValue& obj);
  extern void ToJSON(float val, JSONValue& obj);
  extern void ToJSON(double val, JSONValue& obj);
  extern void ToJSON(const std::string& val, JSONValue& obj);
  // for completeness
  extern void ToJSON(const JSONValue& val, JSONValue& obj);

  /*--------------------------------------------------------------------------------*/
  /** Convert from JSON text to JSON data
   */
  /*--------------------------------------------------------------------------------*/
  extern bool FromJSONString(const std::string& str, JSONValue& obj);

  /*--------------------------------------------------------------------------------*/
  /** Convert from JSON data to JSON text
   */
  /*--------------------------------------------------------------------------------*/
  extern std::string ToJSONString(const JSONValue& obj, bool pretty = true);
};

/*--------------------------------------------------------------------------------*/
/** Interface for converting to and from JSON
 */
/*--------------------------------------------------------------------------------*/
class JSONSerializable
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Return object as JSON object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ToJSON(JSONValue& obj) const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return object as JSON object
   */
  /*--------------------------------------------------------------------------------*/
  virtual JSONValue ToJSON() const {JSONValue obj; ToJSON(obj); return obj;}

  /*--------------------------------------------------------------------------------*/
  /** Set object from JSON
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromJSON(const JSONValue& obj) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  JSONSerializable& operator = (const JSONValue& obj) {FromJSON(obj); return *this;}
  
  /*--------------------------------------------------------------------------------*/
  /** Casting operator
   */
  /*--------------------------------------------------------------------------------*/
  virtual operator JSONValue () const {JSONValue obj; ToJSON(obj); return obj;}
  
  /*--------------------------------------------------------------------------------*/
  /** Parse JSON string
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromJSONString(const std::string& str) {JSONValue obj; json::FromJSONString(str, obj); return FromJSON(obj);}
  
  /*--------------------------------------------------------------------------------*/
  /** Produce JSON string
   */
  /*--------------------------------------------------------------------------------*/
  virtual std::string ToJSONString(bool pretty = true) const {JSONValue obj; ToJSON(obj); return json::ToJSONString(obj, pretty);}
};

namespace json
{
  /*--------------------------------------------------------------------------------*/
  /** Conversion from JSONSerializable based class to JSON
   */
  /*--------------------------------------------------------------------------------*/
  inline void ToJSON(const JSONSerializable& val, JSONValue& obj) {val.ToJSON(obj);}

  /*--------------------------------------------------------------------------------*/
  /** Conversion from JSON to JSONSerializable based class
   */
  /*--------------------------------------------------------------------------------*/
  inline bool FromJSON(const JSONValue& obj, JSONSerializable& val) {return val.FromJSON(obj);}

  /*--------------------------------------------------------------------------------*/
  /** Conversion from [possibly complex] types to JSON value with JSON return (*may* be inefficient)
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  JSONValue ToJSON(const T& val) {JSONValue obj; json::ToJSON(val, obj); return obj;}

  /*--------------------------------------------------------------------------------*/
  /** Templated conversion from [possibly complex] types to JSON string
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  std::string ToJSONString(const T& val, bool pretty = true) {JSONValue obj; json::ToJSON(val, obj); return ToJSONString(obj, pretty);}

  /*--------------------------------------------------------------------------------*/
  /** Templated conversion from [possibly complex] types to JSON string
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  bool FromJSONString(const std::string& str, T& val) {JSONValue obj; return (json::FromJSONString(str, obj) && json::FromJSON(obj, val));}
};
#else
/*--------------------------------------------------------------------------------*/
/** Empty interface to avoid lots of condition class definitions in h files
 */
/*--------------------------------------------------------------------------------*/
class JSONSerializable
{
};
#endif

BBC_AUDIOTOOLBOX_END

#endif
