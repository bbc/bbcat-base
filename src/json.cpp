
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

BBC_AUDIOTOOLBOX_START

namespace json
{
  bool FromJSON(const JSONValue& obj, bool& val)
  {
    bool success = obj.isConvertibleTo(Json::ValueType::booleanValue);
    if (success) val = obj.asBool();
    return success;
  }

  bool FromJSON(const JSONValue& obj, sint_t& val)
  {
    bool success = false;
    if (obj.isConvertibleTo(Json::ValueType::intValue))
    {
#ifdef OLD_JSON_CPP
      // in old versions of the jsonccp library, an int64 value will be convertable
      // to intValue but will thrown an exception when using .asInt(), therefore
      // use .asInt64() and verify value is within int range
      sint64_t temp = obj.asInt64();
      if ((sint_t)temp == temp) // check validity when convert to int
      {
        val = (sint_t)temp;
        success = true;
      }
#else
      val = obj.asInt();
      success = true;
#endif
    }
    return success;
  }

  bool FromJSON(const JSONValue& obj, uint_t& val)
  {
    bool success = false;
    if (obj.isConvertibleTo(Json::ValueType::uintValue))
    {
#ifdef OLD_JSON_CPP
      // in old versions of the jsonccp library, an uint64 value will be convertable
      // to uintValue but will thrown an exception when using .asUInt(), therefore
      // use .asUInt64() and verify value is within int range
      uint64_t temp = obj.asUInt64();
      if ((uint_t)temp == temp) // check validity when convert to int
      {
        val = (uint_t)temp;
        success = true;
      }
#else
      val = obj.asUInt();
      success = true;
#endif
    }
    return success;
  }

  bool FromJSON(const JSONValue& obj, sint64_t& val)
  {
    // in newer versions of the jsoncpp library, an int64 is *not* convertable to int so isNumeric() is used
    bool success = obj.isConvertibleTo(Json::ValueType::intValue) || obj.isNumeric();
    if (success)
    {
      // rare exception cases when value cannot be represented by a *signed* 64-bit int
      try
      {
        val = obj.asInt64();
      }
      catch(...)
      {
        success = false;
      }
    }
    return success;
  }

  bool FromJSON(const JSONValue& obj, uint64_t& val)
  {
    // in newer versions of the jsoncpp library, an uint64 is *not* convertable to uint so isNumeric() is used
    // but value cannot be negative hence the extra asDouble() check
    bool success = obj.isConvertibleTo(Json::ValueType::uintValue) || (obj.isNumeric() && (obj.asDouble() >= 0.0));
    if (success) val = obj.asUInt64();
    return success;
  }

  bool FromJSON(const JSONValue& obj, float& val)
  {
    bool success = obj.isConvertibleTo(Json::ValueType::realValue);
    if (success) val = obj.asFloat();
    return success;
  }

  bool FromJSON(const JSONValue& obj, double& val)
  {
    bool success = obj.isConvertibleTo(Json::ValueType::realValue);
    if (success) val = obj.asDouble();
    return success;
  }

  bool FromJSON(const JSONValue& obj, std::string& val)
  {
    bool success = true;
    if (obj.isString()) val = obj.asString();
    else success = false;
    return success;
  }

  bool FromJSON(const JSONValue& obj, JSONValue& val)
  {
    val = obj;
    return true;
  }

  void ToJSON(bool val, JSONValue& obj)
  {
    obj = val;
  }

  void ToJSON(uint_t val, JSONValue& obj)
  {
    obj = val;
  }

  void ToJSON(sint_t val, JSONValue& obj)
  {
    obj = val;
  }

  void ToJSON(uint64_t val, JSONValue& obj)
  {
    obj = (Json::UInt64)val;
  }

  void ToJSON(sint64_t val, JSONValue& obj)
  {
    obj = (Json::Int64)val;
  }

  void ToJSON(float val, JSONValue& obj)
  {
    obj = val;
  }

  void ToJSON(double val, JSONValue& obj)
  {
    obj = val;
  }

  void ToJSON(const std::string& val, JSONValue& obj)
  {
    obj = val;
  }

  void ToJSON(const JSONValue& val, JSONValue& obj)
  {
    obj = val;
  }

  /*--------------------------------------------------------------------------------*/
  /** Convert from JSON text to JSON data
   */
  /*--------------------------------------------------------------------------------*/
  bool FromJSONString(const std::string& str, JSONValue& obj)
  {
    Json::Reader reader;
    return reader.parse(str, obj);
  }

  /*--------------------------------------------------------------------------------*/
  /** Convert from JSON data to JSON text
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToJSONString(const JSONValue& obj, bool pretty)
  {
    std::string str;

    if (obj.isNull()) str = "{}";
    else if (pretty) str = obj.toStyledString();
    else
    {
      Json::FastWriter writer;
      str = writer.write(obj);
    }

    return str;
  }
};

BBC_AUDIOTOOLBOX_END
