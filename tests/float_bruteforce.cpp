
#include "../const_format.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <random>
#include <bitset>
#include <iomanip>
#include <chrono>


using namespace std::literals::string_view_literals;


union LongDouble
{
    struct {
        uint64_t high;
        uint64_t low;
    };
    unsigned __int128 bits;
    long double val;
};


static_assert(sizeof(LongDouble) == sizeof(long double));


int main()
{
    static constexpr auto format_str = "%f"sv;
    constexpr auto format = cst_fmt::compile_format<format_str, long double>();

    std::random_device rd;
    std::independent_bits_engine<std::mt19937_64, 128, unsigned __int128> rng(rd());

    bool header_printed = false;

    std::cout << std::setprecision(10);

    const auto start = std::chrono::high_resolution_clock::now();

    const unsigned long N = 1000;

    uint32_t count = 0;
    while (count < N) {
        LongDouble a{ .bits = rng() };
        if (std::isnan(a.val)) {
            continue;
        }

        auto res = cst_fmt::format(format, a.val);
        long double parsed = std::strtold(res.cbegin(), nullptr);

        std::stringstream stream;
        stream << a.val;
        long double expected;
        stream >> expected;

        if (expected != parsed) {
            if (!header_printed) {
                std::cout << std::setw(20) << "Value" << "\t" << std::setw(20) << "Result" << "\t" << std::setw(20) << "Expected" << "\t" << std::setw(20) << "Parsed" << "\n";
                header_printed = true;
            }
            //std::cout << std::bitset<64>(a.high) << " " << std::bitset<64>(a.low) << "\t";
            std::cout << std::setw(20) << a.val << "\t" << std::setw(20) << res.view();
            std::cout << "\t" << std::setw(20) << expected << "\t" << std::setw(20) << parsed;
            std::cout << "\n";
        }

        count++;
    }

    const auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time = end - start;

    std::cout << N << " tests done in " << time.count() << " ms \n";
}
