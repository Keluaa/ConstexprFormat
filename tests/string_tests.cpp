
#include <string>

#include "tests.h"


TEST_CASE("%s string_view")
{
    SUBCASE("hello world")
    {
    	static constexpr auto fmt_str = "%s world"sv;
    	static constexpr auto str = "hello"sv;
    	constexpr auto result = cst_fmt::format<fmt_str>(cst_fmt::str_ref<str>{});
    	constexpr auto expected = "hello world"sv;
        CHECK_EQ(result, expected);
    }

    SUBCASE("empty")
    {
        static constexpr auto fmt_str = "%s world"sv;
        static constexpr auto str = ""sv;
    	constexpr auto result = cst_fmt::format<fmt_str>(cst_fmt::str_ref<str>{});
    	constexpr auto expected = " world"sv;
        CHECK_EQ(result, expected);
    }

    SUBCASE("\\0")
    {
        static constexpr auto fmt_str = "%s world"sv;
        static constexpr auto str = "\0"sv;
    	constexpr auto result = cst_fmt::format<fmt_str>(cst_fmt::str_ref<str>{});
    	constexpr auto expected = "\0 world"sv;
        CHECK_EQ(result, expected);
    }
}


TEST_CASE("%s char array")
{
    SUBCASE("hello world")
    {
        static constexpr auto fmt_str = "%s world"sv;
        static constexpr const char str[6] = "hello";
    	constexpr auto result = cst_fmt::format<fmt_str>(cst_fmt::cstr_ref<6, str>{});
    	constexpr auto expected = "hello world"sv;
        CHECK_EQ(result, expected);
    }

    SUBCASE("empty")
    {
        static constexpr auto fmt_str = "%s world"sv;
    	static constexpr const char str[0]{};
    	constexpr auto result = cst_fmt::format<fmt_str>(cst_fmt::cstr_ref<0, str>{});
    	constexpr auto expected = " world"sv;
        CHECK_EQ(result, expected);
    }

    SUBCASE("\\0")
    {
        static constexpr auto fmt_str = "%s world"sv;
        static constexpr const char str[2] = "\0";
    	constexpr auto result = cst_fmt::format<fmt_str>(cst_fmt::cstr_ref<2, str>{});
    	constexpr auto expected = " world"sv;
        CHECK_EQ(result, expected);
    }
}


TEST_CASE("%s dynamic char array")
{
    SUBCASE("hello world")
    {
        static constexpr auto fmt_str = "%s world"sv;
        constexpr auto fmt = cst_fmt::compile_format<fmt_str, cst_fmt::cstr<6>>();
        for (const char* str : { "hello", "bye" }) {
            auto result = cst_fmt::format(fmt, cst_fmt::cstr<6>{str});
            std::string expected = std::string(str) + " world";
            CHECK_EQ(result, expected);
        }
    }

    SUBCASE("empty")
    {
        static constexpr auto fmt_str = "%s world"sv;
        constexpr auto fmt = cst_fmt::compile_format<fmt_str, cst_fmt::cstr<0>>();

        const char* str = "";
        auto result = cst_fmt::format(fmt, cst_fmt::cstr<0>{str});
        std::string expected = std::string(str) + " world";
        CHECK_EQ(result, expected);
    }
}
