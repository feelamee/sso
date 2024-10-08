#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_VOID_CAST_EXPRESSIONS
#include <doctest/doctest.h>

#include <sso/string.hpp>

#include <memory_resource>
#include <ranges>
#include <type_traits>

TEST_SUITE("sso")
{
    TEST_CASE("empty string")
    {
        sso::string s;
        REQUIRE(s.empty());
        REQUIRE_EQ(s.size(), 0);
        REQUIRE_EQ(s.length(), 0);
    }

    TEST_CASE("typename's")
    {
        REQUIRE(requires { typename sso::string::size_type; });
        REQUIRE(requires { typename sso::string::value_type; });
        REQUIRE(requires { typename sso::string::const_reference; });
        REQUIRE(requires { typename sso::string::const_pointer; });
        REQUIRE(requires { typename sso::string::pointer; });
        REQUIRE(requires { typename sso::string::allocator_type; });
        REQUIRE(requires { typename sso::string::string_view; });
        REQUIRE(requires { typename sso::string::reference; });
        REQUIRE(requires { typename sso::string::iterator; });
        REQUIRE(requires { typename sso::string::const_iterator; });
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

    TEST_CASE("copy/move ctor/assignment operator")
    {
        {
            sso::string s;
            sso::string copy(s);
            REQUIRE_EQ(copy, s);

            sso::string another;
            another = copy;
            REQUIRE_EQ(another, copy);
        }

        {
            sso::string s(5, 'a');
            sso::string copy(s);
            REQUIRE_EQ(copy, s);
        }

        {
            sso::string s(5, 'a');
            sso::string copy;
            copy = s;
            REQUIRE_EQ(copy, s);
        }
    }

    TEST_CASE("ctor from `std::string_view`")
    {
        using value_type = char;
        using string_view = std::basic_string_view<value_type>;
        using string = sso::basic_string<value_type>;

        {
            string_view sv;
            string s(sv);
            REQUIRE_EQ(static_cast<string_view>(s), sv);
        }

        {
            string_view sv{ "123" };
            string s(sv);
            REQUIRE_EQ(static_cast<string_view>(s), sv);
        }

        {
            string_view sv{ "123" };
            string s(sv);
            REQUIRE_EQ(s, sv);
        }
    }

    TEST_CASE("access")
    {
        {
            sso::string s{ "hello, world" };
            size_t const i{ 0 };
            REQUIRE(i < s.size());
            REQUIRE_EQ(s[i], 'h');
            REQUIRE_EQ(s.front(), 'h');
            REQUIRE_EQ(s.back(), 'd');
        }

        {
            char const* c_str{ "hello,world" };
            sso::string s{ c_str };
            REQUIRE_EQ(std::strcmp(s.c_str(), c_str), 0);
        }

        {
            char const* c_str{ "" };
            sso::string s{ c_str };
            REQUIRE_EQ(std::strcmp(s.c_str(), c_str), 0);
        }
    }

    TEST_CASE("clear")
    {
        sso::string s{ "hello, world" };
        REQUIRE(!s.empty());
        s.clear();
        REQUIRE(s.empty());
        REQUIRE_EQ(std::strcmp(s.c_str(), ""), 0);
        REQUIRE_EQ(std::strcmp(s.data(), ""), 0);
    }

    TEST_CASE("ctor from temporary object")
    {
        char const* const c_str{ "hello, world" };
        sso::string hello_world{ c_str };

        sso::string s{ std::move(hello_world) };
        REQUIRE_EQ(std::strcmp(s.c_str(), c_str), 0);
    }

    TEST_CASE("at()")
    {
        {
            sso::string s;
            REQUIRE_THROWS_AS(s.at(1), std::out_of_range);
        }
        {
            char const* const hello_world{ "hello, world" };
            sso::string s{ hello_world };
            REQUIRE_EQ(s.at(1), hello_world[1]);
        }
    }

    TEST_CASE("default constructible")
    {
        REQUIRE(std::is_default_constructible_v<sso::string>);
        static_assert(sizeof(sso::string) == 24);
    }

    TEST_CASE("move c-tor")
    {
        std::string_view const _123{ "123" };
        sso::string s{ static_cast<sso::string&&>(sso::string{ _123 }) };

        REQUIRE_EQ(s, _123);
    }

    TEST_CASE_TEMPLATE("sso", CharType, char, char8_t, char16_t, char32_t, wchar_t)
    {
        using string = sso::basic_string<CharType, std::pmr::polymorphic_allocator<CharType>>;
        typename string::allocator_type allocator{ std::pmr::null_memory_resource() };

        std::size_t const max_small_string_size{ 23 / sizeof(CharType) }; // without null-terminator
        REQUIRE_NOTHROW(string{ max_small_string_size, 'a' });
        REQUIRE_THROWS_AS(string(max_small_string_size + 1, 'a', allocator), std::bad_alloc);
    }

    TEST_CASE("reserve")
    {
        using size_type = sso::string::size_type;

        size_type size{ 50 };
        sso::string s{ size, 'x' };
        REQUIRE_GE(s.capacity(), size);

        size_type capacity{ 100 };
        s.reserve(capacity);
        REQUIRE_GE(s.capacity(), capacity);
    }

    TEST_CASE("replace")
    {
        std::string_view const _123{ "123" };
        sso::string s;
        s.replace(0, 0, _123);
        REQUIRE_EQ(s, _123);

        s.replace(0, s.size(), "");
        REQUIRE_EQ(s, "");

        SUBCASE("")
        {
            s.replace(0, 0, _123);
            s.replace(0, 0, _123);
            s.replace(0, 0, _123);
            std::string s123{ _123 };
            REQUIRE_EQ(s, s123 + s123 + s123);
        }
        SUBCASE("")
        {
            s.replace(0, 0, _123);
            s.replace(3, 0, _123);
            s.replace(6, 0, _123);
            std::string s123{ _123 };
            REQUIRE_EQ(s, s123 + s123 + s123);
        }
        SUBCASE("")
        {
            std::string s123{ _123 };
            s.replace(0, 0, s123 + s123 + s123);
            s.replace(3, 6, _123);
            REQUIRE_EQ(s, s123 + s123);
        }
    }

    TEST_CASE("assign")
    {
        char const* const s_123{ "123" };
        sso::string s;

        SUBCASE("")
        {
            s = s_123;
            REQUIRE_EQ(s, s_123);
        }
        SUBCASE("")
        {
            s = std::string_view{ s_123 };
            REQUIRE_EQ(s, s_123);
        }
        SUBCASE("")
        {
            s = sso::string{ s_123 };
            REQUIRE_EQ(s, s_123);
        }
    }

    TEST_CASE("erase")
    {
        sso::string s{ "123" };
        REQUIRE_EQ(s, "123");

        SUBCASE("")
        {
            auto it = s.erase(s.begin());
            REQUIRE_EQ(it, s.begin());
            REQUIRE_EQ(s, "23");

            it = s.erase(s.begin());
            REQUIRE_EQ(it, s.begin());
            REQUIRE_EQ(s, "3");

            it = s.erase(s.begin());
            REQUIRE_EQ(it, s.begin());
            REQUIRE_EQ(s, "");
        }
        SUBCASE("")
        {
            auto it = s.erase(s.begin() + 1);
            REQUIRE_EQ(it, s.begin() + 1);
            REQUIRE_EQ(s, "13");

            it = s.erase(s.begin());
            REQUIRE_EQ(it, s.begin());
            REQUIRE_EQ(s, "3");
        }
        SUBCASE("")
        {
            auto it = s.erase(s.begin(), s.end());
            REQUIRE_EQ(it, s.begin());
            REQUIRE_EQ(s, "");
            REQUIRE(s.empty());
        }
        SUBCASE("")
        {
            auto it = s.erase(s.begin() + 1, s.end());
            REQUIRE_EQ(it, s.end());
            REQUIRE_EQ(s, "1");
        }
    }

    TEST_CASE("push_back")
    {
        sso::string s;
        REQUIRE(s.empty());

        s.push_back('x');
        REQUIRE(!s.empty());
        REQUIRE_EQ(s.back(), 'x');
    }

    TEST_CASE("pop_back")
    {
        sso::string s{ "123" };
        REQUIRE(!s.empty());
        REQUIRE_EQ(s.back(), '3');

        s.pop_back();
        REQUIRE(!s.empty());
        REQUIRE_EQ(s.back(), '2');
    }

    TEST_CASE("append")
    {
        sso::string s123{ "123" };
        sso::string s;
        REQUIRE(s.empty());

        REQUIRE_EQ(s.append(s123), s123);
        REQUIRE_EQ(s.append(s), s123 + s123);
        REQUIRE_EQ(s.append(s), s123 + s123 + s123 + s123);
    }

    TEST_CASE("operator+")
    {
        std::string std{ "123" };
        sso::string sso{ std };
        REQUIRE_EQ(sso, std);
        REQUIRE_EQ(std + std, sso + sso);
        REQUIRE_EQ(std + std + std, sso + sso + sso);
        REQUIRE_EQ(std + sso.c_str(), sso + std);
    }

    TEST_CASE("operator+=")
    {
        std::string const s123{ "123" };
        std::string std{ s123 };
        sso::string sso{ std };
        REQUIRE_EQ(sso, std);
        REQUIRE_EQ(std += s123, sso += s123);

        std += s123;
        std += s123;
        sso += s123;
        sso += s123;
        REQUIRE_EQ(std, sso);
        REQUIRE_EQ(std + sso.c_str(), sso + std);
    }
}
