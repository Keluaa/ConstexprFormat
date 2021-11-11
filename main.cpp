
#include <iostream>

#include "const_format.h"


using namespace std::literals::string_view_literals;

//constexpr auto test_fmt = "hello %d times"sv;
constexpr auto test_fmt = "hello %x times"sv;

/*
 * Formats that should not compile 
 */

//constexpr auto test_fmt = "hello %w times"sv;
//constexpr auto test_fmt = "hello %\0 times"sv;
//constexpr auto test_fmt = "hello times%"sv;


int main()
{
    constexpr cst_fmt::FormattedCharArray arg = cst_fmt::parse_format<test_fmt>(42);
 
  	const std::array<int, 5> tests{
  		1, -1, 10, -10, 0
  	};
  	
  	for (const auto& val_test : tests) {
  		std::cout << "\nval: " << val_test << ":\n";
  		auto str = cst_fmt::parse_format<test_fmt>(val_test);
	    std::cout << "str size: " << str.size() << ", effective size: " << str.effective_size() << "\n";
	    std::cout << "'" << str << "'\n";
  	}
}
