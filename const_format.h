
#ifndef CONSTEXPRFORMAT_CONST_FORMAT_H
#define CONSTEXPRFORMAT_CONST_FORMAT_H


#include <array>
#include <type_traits>
#include <limits>
#include <cstddef>
#include <string_view>
#include <string>
#include <ostream>


namespace cst_fmt
{
    template<size_t N>
    class FormattedCharArray : public std::array<char, N>
    {
        size_t m_effective_size;

    public:
        using std::array<char, N>::data;

        constexpr void set_effective_size(size_t effective_size) { m_effective_size = effective_size; }

        [[nodiscard]]
        constexpr size_t effective_size() const { return m_effective_size; }

        [[nodiscard]]
        constexpr std::string_view view() const { return std::string_view(data(), m_effective_size); }


        [[nodiscard]]
#ifndef __clang__
        // Clang doesn't support constexpr string constructors for now
        constexpr
#endif
        std::string str() const { return std::string(data(), m_effective_size + 1); }
    };
}


template<size_t N>
std::ostream& operator<<(std::ostream& os, const cst_fmt::FormattedCharArray<N>& str)
{
    os << str.view();
    return os;
}


namespace cst_fmt::utils
{
    /**
     *  Returns the number of decimal digits needed to represent the given number.
     */
    constexpr size_t const_log10(std::unsigned_integral auto x)
    {
        size_t i = 0;
        while (x > 0) {
            x /= 10;
            i++;
        }
        return i;
    }


    /**
     *  Returns 10^p.
     */
    template<typename T>
    constexpr T const_10_pow(std::unsigned_integral auto p)
    {
        if (p == 0) {
            return 1;
        }
        else {
            T a = 10;
            for (size_t i = 1; i < p; i++) {
                a *= 10;
            }
            return a;
        }
    }
}


namespace cst_fmt::specialisation
{
    template<char fmt, typename T>
    typename std::enable_if<fmt == 'd' && std::numeric_limits<T>::is_integer, size_t>::type
    consteval formatted_str_length()
    {
        if constexpr (std::is_signed<T>::value) {
            // Add some space a potential minus sign
            return std::numeric_limits<T>::digits10 + 2;
        }
        else {
            return std::numeric_limits<T>::digits10 + 1;
        }
    }


    template<char fmt, typename T>
    typename std::enable_if<fmt == 'd' && !std::numeric_limits<T>::is_integer, size_t>::type
    consteval formatted_str_length()
    {
        static_assert(std::numeric_limits<T>::is_integer, "Unsupported integer type");
        return 0;
    }


    template<char fmt, typename T>
    typename std::enable_if<fmt != 'd', size_t>::type
    consteval formatted_str_length()
    {
        static_assert(fmt == 'd', "Unknown format specifier");
        return 0;
    }


    template<char fmt, size_t N, typename T>
    typename std::enable_if<fmt == 'd', void>::type
    constexpr format_to_str(std::array<char, N>& str, size_t& pos, T val)
    {
        static_assert(std::numeric_limits<T>::is_integer, "Expected an integer type for '%d'");

        if constexpr (std::is_signed<T>::value) {
            if (val < 0) {
                str[pos++] = '-';
                val = -val;
            }
        }

        typedef typename std::make_unsigned<T>::type uT;

        size_t val_digits = utils::const_log10(static_cast<uT>(val));
        uT a = utils::const_10_pow<uT>(val_digits - 1);

        for (size_t i = 0; i < val_digits; i++) {
            str[pos++] = '0' + (val / a);
            val = val % a;
            a /= 10;
        }
    }


    template<char fmt, size_t N, bool>
    typename std::enable_if<fmt == 'd', void>::type
    constexpr format_to_str(std::array<char, N>& str, size_t& pos, bool val)
    {
        str[pos++] = val ? '1' : '0';
    }


    template<char fmt, size_t N, typename T>
    typename std::enable_if<fmt != 'd', void>::type
    constexpr format_to_str(std::array<char, N>& str, size_t& pos, [[maybe_unused]] T val)
    {
        static_assert(fmt == 'd', "Unknown format specifier");
    }
}


namespace cst_fmt::internal
{
    template<const std::string_view& fmt_str, size_t pos>
    consteval size_t next_format()
    {
        constexpr size_t i = fmt_str.find('%', pos);
        if constexpr (i == std::string_view::npos) {
            return std::string_view::npos;
        }

        // Check if there is a character after the '%'
        static_assert(i + 1 < fmt_str.size(), "Missing character after '%'");

        return i + 1;
    }


    template<const std::string_view& fmt, size_t pos>
    consteval size_t get_formatted_str_length()
    {
        [[maybe_unused]]
        constexpr size_t nxt = next_format<fmt, pos>();
        static_assert(nxt == std::string_view::npos, "Not enough arguments for format string");
        return fmt.size() - pos;
    }


    template<const std::string_view& fmt, size_t pos, typename T, typename... Args>
    consteval size_t get_formatted_str_length()
    {
        constexpr size_t nxt = next_format<fmt, pos>();
        static_assert(nxt != std::string_view::npos, "Too many arguments for format string");

        return nxt - pos - 1 // Characters of the format string from the previous format to the next one, excluding the '%'
               + specialisation::formatted_str_length<fmt[nxt], T>() // Maximum length of the formatted type
               + get_formatted_str_length<fmt, nxt + 1, Args...>();
    }


    template<const std::string_view& fmt, typename... Args>
    consteval size_t get_formatted_str_length_start()
    {
        constexpr size_t nxt = next_format<fmt, 0>();

        // We add one for the extra '\0' character

        if constexpr (nxt == std::string_view::npos) {
            static_assert(sizeof...(Args) == 0, "No arguments expected for format string");
            return fmt.size() + 1;
        }
        else {
            static_assert(sizeof...(Args) > 0, "Not enough arguments for format string");
            return get_formatted_str_length<fmt, 0, Args...>() + 1;
        }
    }


    template<const std::string_view& fmt, size_t N, size_t start, size_t end>
    constexpr void copy_fmt_to_array(std::array<char, N>& str, size_t& str_pos)
    {
        constexpr size_t length = end - start;
        static_assert(length >= 0, "Fatal formatting error");
        for (size_t i = 0; i < length; i++) {
            str[str_pos + i] = fmt[start + i];
        }
        str_pos += length;
    }


    template<const std::string_view& fmt, size_t N, size_t pos = 0>
    constexpr void parse_format_internal(std::array<char, N>& str, size_t& str_pos)
    {
        // Copy the rest of the characters of the format string
        copy_fmt_to_array<fmt, N, pos, fmt.size()>(str, str_pos);
    }


    template<const std::string_view& fmt, size_t N, size_t pos = 0, typename T, typename... Args>
    constexpr void parse_format_internal(std::array<char, N>& str, size_t& str_pos, T val, Args... args)
    {
        constexpr size_t nxt = next_format<fmt, pos>();

        // Copy the characters between two format positions to the string, excluding the '%'
        copy_fmt_to_array<fmt, N, pos, nxt - 1>(str, str_pos);

        // Format the value
        specialisation::format_to_str<fmt[nxt]>(str, str_pos, val);

        parse_format_internal<fmt, N, nxt + 1>(str, str_pos, args...);
    }
}


namespace cst_fmt
{
    template<const std::string_view& fmt, typename... Args>
    constexpr auto parse_format(Args... args)
    {
        constexpr size_t str_size = internal::get_formatted_str_length_start<fmt, Args...>();
        FormattedCharArray<str_size> str{};

        size_t str_pos = 0;
        internal::parse_format_internal<fmt, str_size, 0>(str, str_pos, args...);

        str.set_effective_size(str_pos);

        return str;
    }
}


#endif //CONSTEXPRFORMAT_CONST_FORMAT_H
