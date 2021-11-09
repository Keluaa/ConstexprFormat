
#include <iostream>

#include "const_format.h"


using namespace std::literals::string_view_literals;

constexpr auto test_fmt = "hello %d times"sv;


int main()
{
    constexpr cst_fmt::FormattedCharArray arg = cst_fmt::parse_format<test_fmt>(42);

    int a = 42;
    auto str = cst_fmt::parse_format<test_fmt>(a);
    std::cout << "str size: " << str.size() << ", effective size: " << str.effective_size() << "\n";
    std::cout << "'" << str << "'\n";
}
