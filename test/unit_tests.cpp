#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Infrastructure Sanity Check") {
    REQUIRE(1 == 1); // passing test
    REQUIRE(2 == 2); // another passing test
}
