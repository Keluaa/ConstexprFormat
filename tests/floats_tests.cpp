
#include "tests.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <bit>


TEST_CASE("%f")
{
    static constexpr auto format_str = "%f"sv;
    constexpr auto format = cst_fmt::compile_format<format_str, long double>();

    SUBCASE("Big values")
    {
        const long double values[] = {
            -0.1e-1000l, -0.1e-99l,
            -1e-4944l, 1e4932l,
            -2.87305e-2253l, 9.66219e-3792l, 3.093008539e-4932l,
            1.98724e2426l, -6.999997886e1578l,
            4444.4444,
            6666.6666,
            1234.456789
        };

        for (const auto& val : values) {
            const auto result = cst_fmt::format(format, val);
            long double parsed = std::strtold(result.cbegin(), nullptr);

            std::stringstream stream;
            stream << val;
            long double expected;
            stream >> expected;

            // I don't use CHECK_EQ because of a Doctest quirk with long doubles. By quirk, I mean SIGSEGV. I can't reproduce it in another context, however.
            // Also, on failure, Doctest prints the entire number. Even for 1e4000. All 4000 characters. Yes. Why.
            bool res = parsed == expected;
            CHECK(res);
            if (!res) {
                std::cout << "Failed: " << parsed << " == " << expected << "\n";
            }
        }
    }

    SUBCASE("Small values")
    {
        const long double values[] = {
            0,  -0.,
            0.9, 0.09, 0.009, 0.0009, 0.00009, 0.000009,
            1.1, 1.01, 1.001, 1.0001, 1.00001, 1.000001,
            0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001,
            1.234456789e87, 1.000056789e87,
            -0.0001038140947, 0.003002603862,
            4.000004524e-43,
            245474.3012,
        };

        for (const auto& val : values) {
            const auto result = cst_fmt::format(format, val);
            long double parsed = std::strtold(result.cbegin(), nullptr);

            std::stringstream stream;
            stream << val;
            long double expected = std::strtold(stream.rdbuf()->str().c_str(), nullptr);

            bool res = parsed == expected;
            CHECK(res);
            if (!res) {
                std::cout << "Failed: " << parsed << " == " << expected << "\n";
            }
        }
    }

    SUBCASE("Special values")
    {
        const long double values[] = { HUGE_VALL, -HUGE_VALL, NAN, -NAN };
        for (const auto& val : values) {
            const auto result = cst_fmt::format(format, val);
            long double parsed = std::strtold(result.cbegin(), nullptr);

            std::stringstream stream;
            stream << val;
            long double expected = std::strtold(stream.rdbuf()->str().c_str(), nullptr);

            bool res = std::bit_cast<__int128>(parsed) == std::bit_cast<__int128>(expected);
            CHECK(res);
            if (!res) {
                std::cout << "Failed: " << parsed << " == " << expected << "\n";
            }
        }
    }
}
