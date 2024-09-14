#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <sso/string.hpp>

#include <ranges>

TEST_SUITE("sso")
{
    TEST_CASE("empty string")
    {
        sso::string s;
        REQUIRE(s.empty());
        REQUIRE_EQ(s.capacity(), 0);
        REQUIRE_EQ(s.size(), 0);
        REQUIRE_EQ(s.length(), 0);
        REQUIRE_EQ(s.data(), nullptr);
    }

    TEST_CASE("typename's")
    {
        REQUIRE(requires { typename sso::string::size_type; });
        REQUIRE(requires { typename sso::string::value_type; });
        REQUIRE(requires { typename sso::string::const_reference; });
        REQUIRE(requires { typename sso::string::const_pointer; });
        REQUIRE(requires { typename sso::string::pointer; });
        REQUIRE(requires { typename sso::string::allocator_type; });
    }

    TEST_CASE("ctor's")
    {
        using size_type = sso::string::size_type;
        using value_type = sso::string::value_type;
        sso::string s(size_type{ 5 }, value_type{ 'a' });
    }

    TEST_CASE("ctor invariants")
    {
        using size_type = sso::string::size_type;
        using value_type = sso::string::value_type;

        value_type const value{ 'a' };
        size_type const size{ 5 };

        sso::string s(size, value);

        REQUIRE_EQ(s.size(), s.length());
        REQUIRE_EQ(s.size(), size);
        REQUIRE_GE(s.capacity(), s.size());
        REQUIRE_UNARY(s.size() > 0 ? !s.empty() : s.empty());
        for (auto const i : std::views::iota(size_type{ 0 }, s.size()))
        {
            REQUIRE_EQ(s[i], value);
        }
    }

    TEST_CASE("allocator aware")
    {
        using value_type = char;
        using allocator_type = std::allocator<value_type>;
        allocator_type const allocator;
        sso::basic_string<value_type, allocator_type> s1{ allocator };
        sso::basic_string<value_type, allocator_type> s2{ 0, '0', allocator };
        REQUIRE_EQ(s1.get_allocator(), allocator);
    }
}
