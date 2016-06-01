#include "misc.h"

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

BBC_AUDIOTOOLBOX_START
extern bool bbcat_register_bbcat_base();
BBC_AUDIOTOOLBOX_END

USE_BBC_AUDIOTOOLBOX

TEST_CASE("init")
{
  bbcat_register_bbcat_base();
}
