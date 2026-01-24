#include <catch2/catch_test_macros.hpp>

// Forward declarations from src (normally you'd expose these via a header)
int add(int a, int b)
{
    return a + b;
}

TEST_CASE("addition works", "[math]") {
    REQUIRE(add(2, 2) == 4);
    REQUIRE(add(2, 3) == 5);
    REQUIRE(add(-1, 1) == 0);
}

#if 1
TEST_CASE("test 1")
{
    REQUIRE(1 == 1);
}
#endif
