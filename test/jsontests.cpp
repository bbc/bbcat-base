#include <catch/catch.hpp>

#include "json.h"

BBC_AUDIOTOOLBOX_START

TEST_CASE("fromjson")
{
  const char *jsonstr = ("{"
                         "\"a\":1,"
                         "\"b\":-2,"
                         "\"c\":3.4,"
                         //       4294967296
                         "\"d\":-10000000000,"
                         "\"e\": 10000000000,"
                         "\"f\":\"aaa\""
                         "}");

  JSONValue   obj;
  sint_t      sval;
  sint64_t    s64val;
  uint_t      uval;
  uint64_t    u64val;
  float       fval;
  double      dval;
  std::string str = "<unused>";
  
  REQUIRE(json::FromJSONString(jsonstr, obj) == true);

  /*--------------------------------------------------------------------------------*/

  // test unsigned -> signed: should work
  CHECK(json::FromJSON(obj, "a", sval) == true);
  CHECK(sval == 1);

  // test unsigned -> signed 64-bit: should work
  CHECK(json::FromJSON(obj, "a", s64val) == true);
  CHECK(s64val == 1);

  // test unsigned -> unsigned: should work
  CHECK(json::FromJSON(obj, "a", uval) == true);
  CHECK(uval == 1);

  // test unsigned -> unsigned 64-bit: should work
  CHECK(json::FromJSON(obj, "a", u64val) == true);
  CHECK(u64val == 1);

  // test unsigned -> float: should work
  CHECK(json::FromJSON(obj, "a", fval) == true);
  CHECK(fval == 1.f);

  // test unsigned -> double: should work
  CHECK(json::FromJSON(obj, "a", dval) == true);
  CHECK(dval == 1.0);

  // test unsigned -> string: should *fail*
  CHECK(json::FromJSON(obj, "a", str) == false);
  CHECK(str == "<unused>");   // should be left untouched
  
  /*--------------------------------------------------------------------------------*/

  // test signed -> signed: should work
  CHECK(json::FromJSON(obj, "b", sval) == true);
  CHECK(sval == -2);

  // test signed -> signed 64-bit: should work
  CHECK(json::FromJSON(obj, "b", s64val) == true);
  CHECK(s64val == -2);

  // test signed -> unsigned: should *fail*
  CHECK(json::FromJSON(obj, "b", uval) == false);
  CHECK(uval == 1);               // should be left untouched

  // test signed -> unsigned 64-bit: should *fail*
  CHECK(json::FromJSON(obj, "b", u64val) == false);
  CHECK(u64val == 1);             // should be left untouched

  // test signed -> float: should work
  CHECK(json::FromJSON(obj, "b", fval) == true);
  CHECK(fval == -2.f);

  // test signed -> double: should work
  CHECK(json::FromJSON(obj, "b", dval) == true);
  CHECK(dval == -2.0);

  // test signed -> string: should *fail*
  CHECK(json::FromJSON(obj, "b", str) == false);
  CHECK(str == "<unused>");       // should be left untouched

  /*--------------------------------------------------------------------------------*/

  // test float -> signed: should work
  CHECK(json::FromJSON(obj, "c", sval) == true);
  CHECK(sval == 3);

  // test float -> signed 64-bit: should work
  CHECK(json::FromJSON(obj, "c", s64val) == true);
  CHECK(s64val == 3);

  // test float -> unsigned: should work
  CHECK(json::FromJSON(obj, "c", uval) == true);
  CHECK(uval == 3);

  // test float -> unsigned 64-bit: should work
  CHECK(json::FromJSON(obj, "c", u64val) == true);
  CHECK(u64val == 3);

  // test float -> float: should work
  CHECK(json::FromJSON(obj, "c", fval) == true);
  CHECK(fval == 3.4f);

  // test float -> double: should work
  CHECK(json::FromJSON(obj, "c", dval) == true);
  CHECK(dval == 3.4);

  // test float -> string: should *fail*
  CHECK(json::FromJSON(obj, "c", str) == false);
  CHECK(str == "<unused>");   // should be left untouched

  /*--------------------------------------------------------------------------------*/

  // test signed 64-bit -> signed: should *fail*
  CHECK(json::FromJSON(obj, "d", sval) == false);
  CHECK(sval == 3);           // should be left untouched

  // test signed 64-bit -> signed 64-bit: should work
  CHECK(json::FromJSON(obj, "d", s64val) == true);
  CHECK(s64val == -10000000000);

  // test signed 64-bit -> unsigned: should *fail*
  CHECK(json::FromJSON(obj, "d", uval) == false);
  CHECK(uval == 3);           // should be left untouched

  // test signed 64-bit -> unsigned 64-bit: should *fail*
  CHECK(json::FromJSON(obj, "d", u64val) == false);
  CHECK(u64val == 3);         // should be left untouched

  // test signed 64-bit -> float: should work
  CHECK(json::FromJSON(obj, "d", fval) == true);
  CHECK(fval == -10000000000.f);

  // test signed 64-bit -> double: should work
  CHECK(json::FromJSON(obj, "d", dval) == true);
  CHECK(dval == -10000000000.0);

  // test signed 64-bit -> string: should *fail*
  CHECK(json::FromJSON(obj, "d", str) == false);
  CHECK(str == "<unused>");   // should be left untouched

  /*--------------------------------------------------------------------------------*/

  // test unsigned 64-bit -> signed: should *fail*
  CHECK(json::FromJSON(obj, "e", sval) == false);
  CHECK(sval == 3);           // should be left untouched

  // test unsigned 64-bit -> signed 64-bit: should work
  CHECK(json::FromJSON(obj, "e", s64val) == true);
  CHECK(s64val == 10000000000);

  // test unsigned 64-bit -> unsigned: should *fail*
  CHECK(json::FromJSON(obj, "e", uval) == false);
  CHECK(uval == 3);           // should be left untouched

  // test unsigned 64-bit -> unsigned 64-bit: should work
  CHECK(json::FromJSON(obj, "e", u64val) == true);
  CHECK(u64val == 10000000000);

  // test unsigned 64-bit -> float: should work
  CHECK(json::FromJSON(obj, "e", fval) == true);
  CHECK(fval == 10000000000.f);

  // test unsigned 64-bit -> double: should work
  CHECK(json::FromJSON(obj, "e", dval) == true);
  CHECK(dval == 10000000000.0);

  // test unsigned 64-bit -> string: should *fail*
  CHECK(json::FromJSON(obj, "e", str) == false);
  CHECK(str == "<unused>");   // should be left untouched

  /*--------------------------------------------------------------------------------*/

  // test string -> signed: should *fail*
  CHECK(json::FromJSON(obj, "f", sval) == false);
  CHECK(sval == 3);           // should be left untouched

  // test string -> unsigned: should *fail*
  CHECK(json::FromJSON(obj, "f", uval) == false);
  CHECK(uval == 3);           // should be left untouched

  // test string -> float: should *fail*
  CHECK(json::FromJSON(obj, "f", fval) == false);
  CHECK(fval == 10000000000.f);   // should be left untouched

  // test string -> double: should *fail*
  CHECK(json::FromJSON(obj, "f", dval) == false);
  CHECK(dval == 10000000000.0);   // should be left untouched

  // test string -> string: should work
  CHECK(json::FromJSON(obj, "f", str) == true);
  CHECK(str == "aaa");
}

BBC_AUDIOTOOLBOX_END
