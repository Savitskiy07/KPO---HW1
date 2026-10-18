#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("The test target uses C++17 or newer") {
    CHECK(__cplusplus >= 201703L);
}
