
#include "tests.h"


TEST_CASE("%s string_view")
{
    SUBCASE("hello world")
    {
    	constexpr auto fmt_str = "%s world"sv;
    	constexpr auto str = "hello"sv;
    	constexpr auto result = cst_fmt::parse_format<fmt_str>(cst_fmt::str_ref<str>{});
    	constexpr auto expected = "hello world"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("empty")
    {
        constexpr auto fmt_str = "%s world"sv;
    	constexpr auto str = ""sv;
    	constexpr auto result = cst_fmt::parse_format<fmt_str>(cst_fmt::str_ref<str>{});
    	constexpr auto expected = " world"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("\\0")
    {
        constexpr auto fmt_str = "%s world"sv;
    	constexpr auto str = "\0"sv;
    	constexpr auto result = cst_fmt::parse_format<fmt_str>(cst_fmt::str_ref<str>{});
    	constexpr auto expected = "\0 world"sv;
        CHECK_EQ(formatted, expected);
    }
}


TEST_CASE("%s char array")
{
    SUBCASE("hello world")
    {
    	constexpr auto fmt_str = "%s world"sv;
    	constexpr const char str[6] = "hello";
    	constexpr auto result = cst_fmt::parse_format<fmt_str>(cst_fmt::cstr_ref<6, str>{});
    	constexpr auto expected = "hello world"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("empty")
    {
        constexpr auto fmt_str = "%s world"sv;
    	constexpr const char str[0];
    	constexpr auto result = cst_fmt::parse_format<fmt_str>(cst_fmt::cstr_ref<0, str>{});
    	constexpr auto expected = " world"sv;
        CHECK_EQ(formatted, expected);
    }

    SUBCASE("\\0")
    {
        constexpr auto fmt_str = "%s world"sv;
    	constexpr const char str[1] = "\0";
    	constexpr auto result = cst_fmt::parse_format<fmt_str>(cst_fmt::cstr_ref<1, str>{});
    	constexpr auto expected = "\0 world"sv;
        CHECK_EQ(formatted, expected);
    }
}
