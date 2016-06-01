#include <catch/catch.hpp>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

TEST_CASE("stringfrom")
{
  sint16_t s16  = -4364;
  uint16_t u16  =  2131;
  sint32_t s32  = -382567317;
  uint32_t u32  =  395346343;
  sint64_t s64 = -(sint64_t)2987538 * (uint64_t)5436347743;
  uint64_t u64 =  (uint64_t)9579847 * (uint64_t)5436343214;

#if 0
  // use these to generate the test cases!
  BBCDEBUG("  CHECK(StringFrom(s16) == \"%s\");", StringFrom(s16).c_str());
  BBCDEBUG("  CHECK(StringFrom(u16) == \"%s\");", StringFrom(u16).c_str());
  BBCDEBUG("  CHECK(StringFrom(s32) == \"%s\");", StringFrom(s32).c_str());
  BBCDEBUG("  CHECK(StringFrom(u32) == \"%s\");", StringFrom(u32).c_str());
  BBCDEBUG("  CHECK(StringFrom(s64) == \"%s\");", StringFrom(s64).c_str());
  BBCDEBUG("  CHECK(StringFrom(u64) == \"%s\");", StringFrom(u64).c_str());
  BBCDEBUG("  CHECK(StringFrom(s16, \"08\") == \"%s\");", StringFrom(s16, "08").c_str());
  BBCDEBUG("  CHECK(StringFrom(u16, \"08\") == \"%s\");", StringFrom(u16, "08").c_str());
  BBCDEBUG("  CHECK(StringFrom(s32, \"08\") == \"%s\");", StringFrom(s32, "08").c_str());
  BBCDEBUG("  CHECK(StringFrom(u32, \"08\") == \"%s\");", StringFrom(u32, "08").c_str());
  BBCDEBUG("  CHECK(StringFrom(s64, \"08\") == \"%s\");", StringFrom(s64, "08").c_str());
  BBCDEBUG("  CHECK(StringFrom(u64, \"08\") == \"%s\");", StringFrom(u64, "08").c_str());
  BBCDEBUG("  CHECK(StringFrom(s16, \"-8\") == \"%s\");", StringFrom(s16, "-8").c_str());
  BBCDEBUG("  CHECK(StringFrom(u16, \"-8\") == \"%s\");", StringFrom(u16, "-8").c_str());
  BBCDEBUG("  CHECK(StringFrom(s32, \"-8\") == \"%s\");", StringFrom(s32, "-8").c_str());
  BBCDEBUG("  CHECK(StringFrom(u32, \"-8\") == \"%s\");", StringFrom(u32, "-8").c_str());
  BBCDEBUG("  CHECK(StringFrom(s64, \"-8\") == \"%s\");", StringFrom(s64, "-8").c_str());
  BBCDEBUG("  CHECK(StringFrom(u64, \"-8\") == \"%s\");", StringFrom(u64, "-8").c_str());
  BBCDEBUG("  CHECK(StringFrom(s16, \"x\") == \"%s\");", StringFrom(s16, "x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u16, \"x\") == \"%s\");", StringFrom(u16, "x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s32, \"x\") == \"%s\");", StringFrom(s32, "x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u32, \"x\") == \"%s\");", StringFrom(u32, "x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s64, \"x\") == \"%s\");", StringFrom(s64, "x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u64, \"x\") == \"%s\");", StringFrom(u64, "x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s16, \"08x\") == \"%s\");", StringFrom(s16, "08x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u16, \"08x\") == \"%s\");", StringFrom(u16, "08x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s32, \"08x\") == \"%s\");", StringFrom(s32, "08x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u32, \"08x\") == \"%s\");", StringFrom(u32, "08x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s64, \"08x\") == \"%s\");", StringFrom(s64, "08x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u64, \"08x\") == \"%s\");", StringFrom(u64, "08x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s16, \"-8x\") == \"%s\");", StringFrom(s16, "-8x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u16, \"-8x\") == \"%s\");", StringFrom(u16, "-8x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s32, \"-8x\") == \"%s\");", StringFrom(s32, "-8x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u32, \"-8x\") == \"%s\");", StringFrom(u32, "-8x").c_str());
  BBCDEBUG("  CHECK(StringFrom(s64, \"-8x\") == \"%s\");", StringFrom(s64, "-8x").c_str());
  BBCDEBUG("  CHECK(StringFrom(u64, \"-8x\") == \"%s\");", StringFrom(u64, "-8x").c_str());

  BBCDEBUG("  CHECK(StringFrom(M_PI) == \"%s\");", StringFrom(M_PI).c_str());
  BBCDEBUG("  CHECK(StringFrom(M_PI, \"0.4\") == \"%s\");", StringFrom(M_PI, "0.4").c_str());
  BBCDEBUG("  CHECK(StringFrom(M_PI, \"10.4\") == \"%s\");", StringFrom(M_PI, "10.4").c_str());
  BBCDEBUG("  CHECK(StringFrom(M_PI, \"-10.4\") == \"%s\");", StringFrom(M_PI, "-10.4").c_str());
#endif

  CHECK(StringFrom(s16) == "-4364");
  CHECK(StringFrom(u16) == "2131");
  CHECK(StringFrom(s32) == "-382567317");
  CHECK(StringFrom(u32) == "395346343");
  CHECK(StringFrom(s64) == "-16241295463426734");
  CHECK(StringFrom(u64) == "52079336229608258");
  CHECK(StringFrom(s16, "08") == "-0004364");
  CHECK(StringFrom(u16, "08") == "00002131");
  CHECK(StringFrom(s32, "08") == "-382567317");
  CHECK(StringFrom(u32, "08") == "395346343");
  CHECK(StringFrom(s64, "08") == "-16241295463426734");
  CHECK(StringFrom(u64, "08") == "52079336229608258");
  CHECK(StringFrom(s16, "-8") == "-4364   ");
  CHECK(StringFrom(u16, "-8") == "2131    ");
  CHECK(StringFrom(s32, "-8") == "-382567317");
  CHECK(StringFrom(u32, "-8") == "395346343");
  CHECK(StringFrom(s64, "-8") == "-16241295463426734");
  CHECK(StringFrom(u64, "-8") == "52079336229608258");
  CHECK(StringFrom(s16, "x") == "ffffeef4");
  CHECK(StringFrom(u16, "x") == "853");
  CHECK(StringFrom(s32, "x") == "e9327c6b");
  CHECK(StringFrom(u32, "x") == "179081a7");
  CHECK(StringFrom(s64, "x") == "ffc64ca0b935d552");
  CHECK(StringFrom(u64, "x") == "b905e1601a9b42");
  CHECK(StringFrom(s16, "08x") == "ffffeef4");
  CHECK(StringFrom(u16, "08x") == "00000853");
  CHECK(StringFrom(s32, "08x") == "e9327c6b");
  CHECK(StringFrom(u32, "08x") == "179081a7");
  CHECK(StringFrom(s64, "08x") == "ffc64ca0b935d552");
  CHECK(StringFrom(u64, "08x") == "b905e1601a9b42");
  CHECK(StringFrom(s16, "-8x") == "ffffeef4");
  CHECK(StringFrom(u16, "-8x") == "853     ");
  CHECK(StringFrom(s32, "-8x") == "e9327c6b");
  CHECK(StringFrom(u32, "-8x") == "179081a7");
  CHECK(StringFrom(s64, "-8x") == "ffc64ca0b935d552");
  CHECK(StringFrom(u64, "-8x") == "b905e1601a9b42");
  CHECK(StringFrom(M_PI) == "3.14159265358979311599796346854419");
  CHECK(StringFrom(M_PI, "0.4") == "3.1416");
  CHECK(StringFrom(M_PI, "10.4") == "    3.1416");
  CHECK(StringFrom(M_PI, "-10.4") == "3.1416    ");
}

BBC_AUDIOTOOLBOX_END
