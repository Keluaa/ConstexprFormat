
#include <iostream>

#include "../const_format.h"

using namespace std::literals::string_view_literals;


#ifndef TEST_NUMBER
#error "A test number should be specified."
#endif


#define FORMAT_ARGS val_test


#if TEST_NUMBER == 0

// '%d' number format
constexpr auto test_fmt = "A number: %d"sv;
constexpr auto val_test = 42;
constexpr auto expected = "A number: 42"sv;

#elif TEST_NUMBER == 1

// '%x' number format
constexpr auto test_fmt = "A hex number: %x"sv;
constexpr auto val_test = 42;
constexpr auto expected = "A hex number: 0x2A"sv;

#elif TEST_NUMBER == 2

// Empty format
constexpr auto test_fmt = ""sv;
constexpr auto expected = ""sv;

#undef FORMAT_ARGS
#define FORMAT_ARGS

#elif TEST_NUMBER == 3

// '%s' string format with a string view
constexpr auto val_view = "hello"sv;
constexpr cst_fmt::str_ref<val_view> val_test;
constexpr auto test_fmt = "%s world"sv;
constexpr auto expected = "hello world"sv;

#elif TEST_NUMBER == 4

// '%s' string format with a char array
constexpr const char val_cstr[6] = "hello";
constexpr cst_fmt::cstr_ref<6, val_cstr> val_test;
constexpr auto test_fmt = "%s world"sv;
constexpr auto expected = "hello world"sv;

#elif TEST_NUMBER == 5

// '%s' string format with a non-const char array
typedef cst_fmt::cstr<6> val_test;
constexpr auto test_fmt = "%s world"sv;

#define ONLY_COMPILE

#else
#error "Unknown test number: " TEST_NUMBER
#endif


int main()
{
#ifndef ONLY_COMPILE
    constexpr auto str = cst_fmt::parse_format<test_fmt>(FORMAT_ARGS);
    std::cout << "'" <<  str << "'\n";
    static_assert(str == expected);
    return str == expected;
#else
    constexpr auto str = cst_fmt::compile_format_string<test_fmt, FORMAT_ARGS>();
#endif
}
