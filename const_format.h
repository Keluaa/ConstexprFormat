
#ifndef CONSTEXPRFORMAT_CONST_FORMAT_H
#define CONSTEXPRFORMAT_CONST_FORMAT_H


#include <array>
#include <type_traits>
#include <concepts>
#include <limits>
#include <cstddef>
#include <string_view>
#include <string>
#include <ostream>


namespace cst_fmt::utils
{
    /**
     * Type trait for const iterable types with a size.
     */
    template <typename T, typename = void>
    struct is_const_iterable : std::false_type {};
    template <typename T>
    struct is_const_iterable<T,
            std::void_t<decltype(std::declval<T>().cbegin()),
                        decltype(std::declval<T>().cend()),
                        decltype(std::declval<T>().size())>>
            : std::true_type {};


    /**
     *  Returns the number of decimal digits needed to represent the given number.
     */
    template<typename T>
    constexpr size_t const_log10(T x)
    {
        static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "The argument must be an unsigned integral");
        size_t i = 0;
        while (x > 0) {
            x /= 10;
            i++;
        }
        return i;
    }


    /**
     *  Returns the number of decimal digits needed to represent the given number in hexadecimal.
     */
    template<typename T>
    constexpr size_t const_log16(T x)
    {
        static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "The argument must be an unsigned integral");
        size_t i = 0;
        while (x > 0) {
            x >>= 4;
            i++;
        }
        return i;
    }


    /**
     *  Returns 10^p. Integers only.
     */
    template<typename R, typename T>
    constexpr R const_10_pow(T p)
    {
        static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "The argument must be an unsigned integral");
        if (p == 0) {
            return 1;
        }
        else {
            R a = 10;
            for (size_t i = 1; i < p; i++) {
                a *= 10;
            }
            return a;
        }
    }


    /**
     *  Returns 16^p. Integers only.
     */
    template<typename R, typename T>
    constexpr R const_16_pow(T p)
    {
        static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "The argument must be an unsigned integral");
        if (p == 0) {
            return 1;
        }
        else {
            return R(16) << (4 * (p - 1));
        }
    }
}


namespace cst_fmt
{
    /**
     * A simple struct holding reusable information for a format.
     */
    template<const std::string_view& fmt, const size_t N>
    struct CompiledFormat
    {
        [[nodiscard]]
        constexpr const std::string_view& get_fmt() const { return fmt; }

        [[nodiscard]]
        constexpr size_t get_str_size() const { return N; }
    };


    /**
     * A constant size, string-like object.
     *
     */
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


        template<typename T>
        typename std::enable_if<utils::is_const_iterable<T>::value, bool>::type
        constexpr operator==(const T& array) const
        {
            if (m_effective_size != array.size()) {
                return false;
            }
            // We iterate on the other array since our array size is bigger than the effective size
            return std::equal(array.cbegin(), array.cend(), this->cbegin());
        }


        template<typename T>
        typename std::enable_if<utils::is_const_iterable<T>::value, bool>::type
        constexpr operator!=(const T& array) const
        {
            if (m_effective_size != array.size()) {
                return true;
            }
            // We iterate on the other array since our array size is bigger than the effective size
            return !std::equal(array.cbegin(), array.cend(), this->cbegin());
        }
    };


    template<size_t N>
    std::ostream& operator<<(std::ostream& os, const FormattedCharArray<N>& str)
    {
        os << str.view();
        return os;
    }
}


/*
 * Additional formats can be specified here by using the same structure.
 */
namespace cst_fmt::specialisation
{
	//
	// %d -> decimal number
	//
	
    template<char fmt, typename T>
    typename std::enable_if<fmt == 'd' && std::numeric_limits<T>::is_integer, size_t>::type
    consteval formatted_str_length()
    {
        if constexpr (std::is_signed<T>::value) {
            // Add one space for a potential minus sign
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


    template<char fmt, size_t N, typename T>
    typename std::enable_if<fmt == 'd' && !std::is_same_v<T, bool>, void>::type
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

        size_t val_digits = 0;
        
        if (val == 0) {
        	val_digits = 1;
        }
        else {
        	val_digits = utils::const_log10(static_cast<uT>(val));
        }
        
        uT a = utils::const_10_pow<uT>(val_digits - 1);

        for (size_t i = 0; i < val_digits; i++) {
            str[pos++] = '0' + (val / a);
            val = val % a;
            a /= 10;
        }
    }


    template<char fmt, size_t N, typename T>
    typename std::enable_if<fmt == 'd' && std::is_same_v<T, bool>, void>::type
    constexpr format_to_str(std::array<char, N>& str, size_t& pos, T val)
    {
        str[pos++] = val ? '1' : '0';
    }
    
    
    //
    // %x -> hexadecimal number
    //
    
    
    template<char fmt, typename T>
    typename std::enable_if<fmt == 'x' && std::numeric_limits<T>::is_integer, size_t>::type
    consteval formatted_str_length()
    {
    	// +2 for the '0x' prefix
        return 2 + std::numeric_limits<T>::digits / 4 + 1;
    }
    
    
    template<char fmt, size_t N, typename T>
    typename std::enable_if<fmt == 'x' && !std::is_same_v<T, bool>, void>::type
    constexpr format_to_str(std::array<char, N>& str, size_t& pos, T val)
    {
        static_assert(std::numeric_limits<T>::is_integer, "Expected an integer type for '%x'");

        typedef typename std::make_unsigned<T>::type uT;

        uT u_val = static_cast<uT>(val);
        size_t val_digits = 0;
        
        if (val == 0) {
        	val_digits = 1;
        }
        else {
        	val_digits = utils::const_log16(u_val);
        }
        
        uT a = utils::const_16_pow<uT>(val_digits - 1);
        
        str[pos++] = '0';
        str[pos++] = 'x';
        for (size_t i = 0; i < val_digits; i++) {
        	char c = '0' + (u_val / a);
        	if (c > '9') {
        		c += 7; // Shift to the 'A' to 'F' range
        	}
            str[pos++] = c;
            u_val = u_val % a;
            a /= 16;
        }
    }


    template<char fmt, size_t N, typename T>
    typename std::enable_if<fmt == 'x' && std::is_same_v<T, bool>, void>::type
    constexpr format_to_str(std::array<char, N>& str, size_t& pos, T val)
    {
        str[pos++] = '0';
        str[pos++] = 'x';
        str[pos++] = val ? '1' : '0';
    }
    
    
    // 
    // %s -> string-like objects
    //
    // Only 'char[N]' and 'string_view' of static char arrays are supported.
    // The length of the string must be known at compile time.
    // 
    
    
    // char array
    
    template<char fmt, typename T>
    typename std::enable_if<fmt == 's' 
    		&& std::is_bounded_array_v<T>
    		&& std::is_same_v<
    			typename std::remove_cv<typename std::remove_extent<T>::type>::type, 
    			char>,
    	size_t>::type
    consteval formatted_str_length()
    {
    	return std::extent_v<T>;
    }
    
    
    template<char fmt, typename T, typename... Args>
    [[maybe_unused]]
    typename std::enable_if<fmt == 's' 
    	&& !(std::is_bounded_array_v<T> 
    		&& std::is_same_v<
    				typename std::remove_cv<typename std::remove_extent<T>::type>::type, 
    			char>),
    	size_t>::type
    consteval formatted_str_length(Args...)
    {
        static_assert(fmt == '\0', "'%s' accepts only bounded arrays of char with known size");
        return 0;
    }
    
    
    template<char fmt, size_t N, typename T>
    typename std::enable_if<fmt == 's' 
    		&& std::is_bounded_array_v<T>
    		&& std::is_same_v<
    			typename std::remove_cv<typename std::remove_extent<T>::type>::type, 
    			char>,
    	void>::type
    constexpr format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
    	constexpr size_t STR_LEN = std::extent_v<T>;
    	if constexpr (STR_LEN > 0) {
    		for (size_t i = 0; i < STR_LEN - 1; i++) {
	            str[pos + i] = val[i];
	        }
	        
	        // Don't copy the '\0' at the end of the array
	        if (val[STR_LEN - 1] != '\0') {
	        	str[pos + STR_LEN - 1] = val[STR_LEN - 1];
	        	pos += STR_LEN;
	        }
	        else {
	        	pos += STR_LEN - 1;
	        }
    	}
    }
    
    
    template<char fmt, size_t N, typename T>
    [[maybe_unused]]
    typename std::enable_if<fmt == 's' 
    		&& !(std::is_bounded_array_v<T>
    		&& std::is_same_v<
    			typename std::remove_cv<typename std::remove_extent<T>::type>::type, 
    			char>),
    	void>::type
    constexpr format_to_str(...)
    {
  		static_assert(fmt == '\0', "'%s' accepts only bounded arrays of char with known size");
    }
    
    
    /**
     * Default option, used only when 'fmt' is not specified by another definition.
     * Made to fail in all cases.
     */
    template<char fmt, typename T, typename... Args>
    [[maybe_unused]]
    consteval size_t formatted_str_length(Args...)
    {
        static_assert(fmt == '\0', "Unknown format specifier");
        static_assert(fmt != '\0', "'\0' is not a valid format specifier");
        return 0;
    }

    
    /**
     * Default option, used only when 'fmt' is not specified by another definition.
     * Made to fail in all cases.
     */
    template<char fmt>
    [[maybe_unused]]
    constexpr void format_to_str(...)
    {
        static_assert(fmt == '\0', "Unknown format specifier");
        static_assert(fmt != '\0', "'\0' is not a valid format specifier");
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
        if constexpr (fmt.empty()) {
            static_assert(sizeof...(Args) == 0, "Expected no arguments for empty format string");
            return 1;
        }
        else {
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
    // TODO : check if this could actually reduce compilation time, since we are using templates, functions with the same template params doesn't need to be compiled at each invocation right?
    template<const std::string_view& fmt, typename... Args>
    consteval auto compile_format_string()
    {
        constexpr size_t str_size = internal::get_formatted_str_length_start<fmt, Args...>();
        return CompiledFormat<fmt, str_size>{};
    }


    template<const std::string_view& fmt, size_t str_size, typename... Args>
    constexpr auto format([[maybe_unused]] CompiledFormat<fmt, str_size> compiled_format, Args... args)
    {
        FormattedCharArray<str_size> str{};

        size_t str_pos = 0;
        internal::parse_format_internal<fmt, str_size, 0>(str, str_pos, args...);
        str.set_effective_size(str_pos);

        return str;
    }


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
