# ConstexprFormat

---

Simple and lightweight compile-time string formatting using c++20 concepts.

It has two main usages:
 - knowing what is the maximum length of a formatted string at compile time
 - creating a formatted constant string at compile time

**This is mostly an exercise for myself, I don't recommend using it in general**.

Formatting options (`%0.3f` or `%+d`) are not supported. This is not a `sprintf` alternative.

## Example
```cpp
#include <iostream>
#include "const_format.h"

constexpr std::string_view format = "There is %d characters in this string.";
constexpr std::size_t length = format.size();
constexpr auto formatted = cst_fmt::format<format>(length); // resulting type is cst_fmt::FormattedCharArray<57>

constexpr std::string_view other_format = "This is a string: '%s', of length %d";
constexpr auto compiled_format = cst_fmt::compile_format<other_format, cst_fmt::cstr<100>, size_t>(); // The '%s' will be 100 characters at max

consteval auto test_compiled_format(const char* str, size_t str_length)
{
    return cst_fmt::format(compiled_format, cst_fmt::cstr<100>{str}, str_length);
}

int main()
{
    std::cout << formatted << std::endl;
    std::cout << "Effective size: " << formatted.effective_size() << std::endl;
    std::cout << "Real size in memory: " << formatted.size() << std::endl; // cst_fmt::FormattedCharArray inherits from std::array
    
    std::cout << "Compiled format result:\n" << test_compiled_format(formatted.cstr(), length) << std::endl;
}
```

## Supported formats
- `%d` : signed/unsigned integer number in decimal (supports booleans)
- `%x` : signed/unsigned integer number in hexadecimal (supports booleans)
- `%s` : string view, `std::string_view str`, encapsulated in `cst_fmt::str_ref<str>`
- `%s` : char array, `char str[N]`, encapsulated in `cst_fmt::cstr_ref<N, str>`
- `%s` : dynamic string, `std::string str`, `char* str`, encapsulated in `cst_fmt::cstr<N>{str}` (or `str.cstr()`), with `N` the maximum length of the string.
- `%c` : character
- `%f` : float, double, long double (not very stable)

**Note: %f is only supported with gcc, as it seems to be the only compiler with a constexpr math library right now.**


## How it works

It works in three main steps:
- calculation of what would be the maximum length the of the formatted string
- creation of a char array-like object using the maximum string length as the size
- filling of the char array-like object

Since the first step is independent of the two others, it this one which is executed when a format string is compiled using 'cst_fmt::compile_format'.

Adding a new format, or supporting an additional type, requires only two new template specialisations : 
 - `cst_fmt::specialisation::formatted_str_length` to get the maximum length of the format
 - `cst_fmt::specialisation::format_to_str` to transform a value into characters
