
#include <iostream>

#include "../const_format.h"


using namespace std::literals::string_view_literals;


#ifndef TEST_NUMBER
#warning "A test number should be specified."
// Valid format to make the test fail
constexpr auto test_fmt = "A number: %d"sv;
constexpr auto val_test = 42;
#endif


#define FORMAT_ARGS val_test


#if TEST_NUMBER == 0

// Unknown format
constexpr auto test_fmt = "A number: %w"sv;
constexpr auto val_test = 42;

#elif TEST_NUMBER == 1

// '\0' as a format
constexpr auto test_fmt = "A number: %\0"sv;
constexpr auto val_test = 42;

#elif TEST_NUMBER == 2

// Missing character after '%'
constexpr auto test_fmt = "A number: %"sv;
constexpr auto val_test = 42;

#elif TEST_NUMBER == 3

// Empty format with at least one parameter
constexpr auto test_fmt = ""sv;
constexpr auto val_test = 42;

#else
#warning "Unknown test number"
// Valid format to make the test fail
constexpr auto test_fmt = "A number: %d"sv;
constexpr auto val_test = 42;
#endif


int main()
{
    constexpr auto str = cst_fmt::parse_format<test_fmt>(FORMAT_ARGS);
    std::cout << str << "\n";
    return 0;
}
