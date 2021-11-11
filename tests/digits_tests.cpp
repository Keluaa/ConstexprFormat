
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../const_format.h"

using namespace std::literals::string_view_literals;


TEST_CASE("%d format")
{
    static constexpr auto fmt_str_simple_d = "A number: %d"sv;
    static constexpr auto fmt_simple_d = cst_fmt::compile_format_string<fmt_str_simple_d, int>();

    SUBCASE("positive")
    {
        const int a = 42;
        constexpr auto formatted = cst_fmt::format(fmt_simple_d, a);
        constexpr auto expected = "A number: 42"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("negative")
    {
        const int a = -42;
        constexpr auto formatted = cst_fmt::format(fmt_simple_d, a);
        constexpr auto expected = "A number: -42"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("zero")
    {
        const int a = 0;
        constexpr auto formatted = cst_fmt::format(fmt_simple_d, a);
        constexpr auto expected = "A number: 0"sv;
        CHECK_EQ(formatted, expected);
    }
}


TEST_CASE("%x format")
{
    static constexpr auto fmt_str_simple_x = "A hex number: %x"sv;
    static constexpr auto fmt_simple_x = cst_fmt::compile_format_string<fmt_str_simple_x, int>();

    SUBCASE("positive")
    {
        const int a = 42;
        constexpr auto formatted = cst_fmt::format(fmt_simple_x, a);
        constexpr auto expected = "A hex number: 0x2A"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("negative")
    {
        const int a = -42;
        constexpr auto formatted = cst_fmt::format(fmt_simple_x, a);
        constexpr auto expected = "A hex number: 0xFFFFFFD6"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("zero")
    {
        const int a = 0;
        constexpr auto formatted = cst_fmt::format(fmt_simple_x, a);
        constexpr auto expected = "A hex number: 0x0"sv;
        CHECK_EQ(formatted, expected);
    }
}


TEST_CASE("bool formats")
{
    SUBCASE("%d true")
    {
        static constexpr auto fmt_str_simple_d = "A number: %d"sv;
        constexpr auto formatted = cst_fmt::parse_format<fmt_str_simple_d>(true);
        constexpr auto expected = "A number: 1"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("%d false")
    {
        static constexpr auto fmt_str_simple_d = "A number: %d"sv;
        constexpr auto formatted = cst_fmt::parse_format<fmt_str_simple_d>(false);
        constexpr auto expected = "A number: 0"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("%x true")
    {
        static constexpr auto fmt_str_simple_x = "A hex number: %x"sv;
        constexpr auto formatted = cst_fmt::parse_format<fmt_str_simple_x>(true);
        constexpr auto expected = "A hex number: 0x1"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("%x false")
    {
        static constexpr auto fmt_str_simple_x = "A hex number: %x"sv;
        constexpr auto formatted = cst_fmt::parse_format<fmt_str_simple_x>(false);
        constexpr auto expected = "A hex number: 0x0"sv;
        CHECK_EQ(formatted, expected);
    }
}
