
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

#else
#error "Unknown test number: " TEST_NUMBER
#endif


int main()
{
    constexpr auto str = cst_fmt::parse_format<test_fmt>(FORMAT_ARGS);
    std::cout << "'" <<  str << "'\n";
    return str == expected;
}
