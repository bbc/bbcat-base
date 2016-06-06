#include "misc.h"
#include "register.h"

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

USE_BBC_AUDIOTOOLBOX

TEST_CASE("init")
{
  bbcat_register_bbcat_base();
}
