#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <sso/string.hpp>

TEST_SUITE("sso")
{
    TEST_CASE("empty string")
    {
        sso::string s;
        REQUIRE(s.empty());
        REQUIRE_EQ(s.capacity(), 0);
        REQUIRE_EQ(s.size(), 0);
        REQUIRE_EQ(s.length(), 0);
    }

    TEST_CASE("typename's")
    {
        REQUIRE(requires { typename sso::string::size_type; });
        REQUIRE(requires { typename sso::string::value_type; });
    }
}
