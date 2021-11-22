
#ifndef CONSTEXPRFORMAT_CONST_FORMAT_H
#define CONSTEXPRFORMAT_CONST_FORMAT_H


#include <array>
#include <type_traits>
#include <concepts>
#include <limits>
#include <bit>
#include <tuple>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <string_view>
#include <string>
#include <ostream>


namespace cst_fmt::utils
{
	template<size_t N, const char (&STR)[N]>
	struct CharArrayHolder
	{
		static constexpr bool _is_char_array_holder = true;
		static constexpr size_t size() { return N; }
		static constexpr const char* get() { return STR; }
	};
	
	
	template<typename T>
	concept is_char_array_holder = requires {
		T::_is_char_array_holder == true;
	};
	
	
	template<const std::string_view& STR>
	struct StrViewHolder
	{
		static constexpr bool _is_str_view_holder = true;
		static constexpr const std::string_view& get() { return STR; }
	};
	
	
	template<typename T>
	concept is_str_view_holder = requires {
		T::_is_str_view_holder == true;
	};
	
	
	template<size_t N>
	struct DynStrHolder
	{
		static constexpr bool _is_dyn_str_holder = true;
		static constexpr size_t size() { return N; }
		
		const char* str;
	};
	
	
	template<typename T>
	concept is_dyn_str_holder = requires {
		T::_is_dyn_str_holder == true;
	};
	

    template<typename T>
    concept is_const_iterable = requires (const T& val) {
        val.cbegin();
        val.cend();
        val.size();
    };


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
    constexpr size_t const_log16(T x) // TODO : change return type to uint8_t
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
    
    
    /**
     *  Converts the given number to characters in base 10.
     */
    template<size_t N, typename T>
    void int_to_char_array(std::array<char, N>& str, size_t& pos, const T& val)
    {
    	if constexpr (std::is_signed_v<T>) {
    		if (val < 0) {
    			str[pos++] = '-';
    		}
    	}
    	
    	typedef typename std::make_unsigned<T>::type uT;
    	uT u_val = static_cast<uT>(val);

        uint8_t val_digits = 0;
        
        if (val == 0) {
        	val_digits = 1;
        }
        else {
        	val_digits = utils::const_log10(u_val);
        }
        
        uT a = utils::const_10_pow<uT>(val_digits - 1);

        for (uint8_t i = 0; i < val_digits; i++) {
            str[pos++] = '0' + (u_val / a);
            u_val = u_val % a;
            a /= 10;
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
        static constexpr const std::string_view& get_fmt() { return fmt; }

        [[nodiscard]]
        static constexpr size_t get_str_size() { return N; }
    };


    /**
     * A constant size, string-like object.
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
        constexpr const char* cstr() const { return data(); }

        [[nodiscard]]
        constexpr std::string_view view() const { return std::string_view(data(), m_effective_size); }


        [[nodiscard]]
#ifndef __clang__
        // Clang doesn't support constexpr string constructors for now
        constexpr
#endif
        std::string str() const { return std::string(data(), m_effective_size + 1); }


        template<typename T>
            requires utils::is_const_iterable<T>
        constexpr bool operator==(const T& array) const
        {
            if (m_effective_size != array.size()) {
                return false;
            }
            // We iterate on the other array since our array size is bigger than the effective size
            return std::equal(array.cbegin(), array.cend(), this->cbegin());
        }


        template<typename T>
            requires utils::is_const_iterable<T>
        constexpr bool operator!=(const T& array) const
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
	
	
	template<char fmt>
	concept decimal_format = fmt == 'd';
	
	
    template<char fmt, typename T>
    	requires decimal_format<fmt> && std::is_integral_v<T>
    consteval size_t formatted_str_length()
    {
        if constexpr (std::is_signed_v<T>) {
            // Add one space for a potential minus sign
            return std::numeric_limits<T>::digits10 + 2;
        }
        else {
            return std::numeric_limits<T>::digits10 + 1;
        }
    }


    template<char fmt, typename T>
    	requires decimal_format<fmt> && (!std::is_integral_v<T>)
    consteval size_t formatted_str_length()
    {
        static_assert(fmt == '\0', "'%d' expected an integral type");
        return 0;
    }


    template<char fmt, size_t N, typename T>
    	requires decimal_format<fmt> && std::is_integral_v<T> && (!std::same_as<T, bool>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, T val)
    {
        utils::int_to_char_array(str, pos, val);
    }


    template<char fmt, size_t N, typename T>
     	requires decimal_format<fmt> && std::is_integral_v<T> && std::same_as<T, bool>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
        str[pos++] = val ? '1' : '0';
    }
    
    
    template<char fmt, size_t N, typename T>
     	requires decimal_format<fmt> && (!std::is_integral_v<T>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
    	static_assert(fmt == '\0', "'%d' expected an integral type");
    }
    
    
    //
    // %x -> hexadecimal number
    //
    
    
	template<char fmt>
	concept hex_format = fmt == 'x';
	
    
    template<char fmt, typename T>
    	requires hex_format<fmt> && std::is_integral_v<T>
    consteval size_t formatted_str_length()
    {
    	// +2 for the '0x' prefix
        return 2 + std::numeric_limits<T>::digits / 4 + 1;
    }
    
    
    template<char fmt, typename T>
    	requires hex_format<fmt> && (!std::is_integral_v<T>)
    consteval size_t formatted_str_length()
    {
        static_assert(fmt == '\0', "'%x' expected an integral type");
        return 0;
    }
    
    
    template<char fmt, size_t N, typename T>
    	requires hex_format<fmt> && std::is_integral_v<T> && (!std::same_as<T, bool>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
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
     	requires hex_format<fmt> && std::is_integral_v<T> && std::same_as<T, bool>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
        str[pos++] = '0';
        str[pos++] = 'x';
        str[pos++] = val ? '1' : '0';
    }
    
    
    template<char fmt, size_t N, typename T>
     	requires hex_format<fmt> && (!std::is_integral_v<T>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
    	static_assert(fmt == '\0', "'%x' expected an integral type");
    }


    //
    // %f -> floating point numbers
    //


    template<char fmt>
    concept float_format = fmt == 'f';


    template<char fmt, typename T>
        requires float_format<fmt> && std::is_floating_point_v<T>
    consteval size_t formatted_str_length()
    {
        // sign + comma + number of digits for exact representation + exponent 'e' + exponent sign + exponent length
        return 1 + 1 + std::numeric_limits<T>::max_digits10 + 1 + 1 + utils::const_log10(std::numeric_limits<T>::max_exponent10);
    }


    template<char fmt, typename T>
        requires float_format<fmt> && (!std::is_floating_point_v<T>)
    consteval size_t formatted_str_length()
    {
        static_assert(fmt == '\0', "'%f' expected an floating point type");
        return 0;
    }
    
    
    template<char fmt, size_t N, typename T>
        requires float_format<fmt> && std::is_floating_point_v<T>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
        // Handle edge cases: NaN and inf. 0 is also optimized.
	    if (std::isnan(val)) {
	    	str[pos++] = 'N';
	    	str[pos++] = 'a';
	    	str[pos++] = 'N';
	        return;
	    }
	    
	    if (val == 0) {
	    	str[pos++] = '0';
	        return;
	    }
	
	    if (val < 0) {
	    	str[pos++] = '-';
	    }
	
	    if (std::isinf(val)) {
	    	str[pos++] = 'i';
	    	str[pos++] = 'n';
	    	str[pos++] = 'f';
	        return;
	    }
	    
	    const T val_abs = std::abs(val);
	
	    // Get the base 10 exponent and mantissa
	    double raw_exp = std::log10(val_abs);
	    int16_t exp = std::floor(std::abs(raw_exp)) * (raw_exp < 0 ? -1 : 1);
	
	    T mantissa = val_abs / std::pow(10, exp);
	
	    const uint8_t max_digits = 6;
	    uint8_t digits = 0;
	
	    // Write the integer part of the mantissa
	    if (std::numeric_limits<T>::has_denorm && val_abs >= std::numeric_limits<T>::min()) {
	        // TODO : maybe unuseful
	        uint64_t int_mantissa = mantissa;
	        mantissa -= int_mantissa;
	
	        digits += const_log10(int_mantissa);
	
	    	utils::int_to_char_array(str, pos, int_mantissa);
	    }
	    else {
	    	str[pos++] = '0';
	    }

	    str[pos++] = '.';
	
	    // Write the decimal psrt of the mantissa. The last digit to write is rounded.
	    while (mantissa != 0 && digits < max_digits) {
	        mantissa *= 10;
	        uint8_t digit_val = mantissa;
	        mantissa -= digit_val;
	        str[pos++] = '0' + digit_val;
	        digits++;
	
	        if (uint16_t((max_digits - digits) * mantissa) == 0) {
	            break;
	        }
	    }
	
	    if (mantissa != 0) {
	        uint8_t digit_val = std::round(mantissa * 10);
	        str[pos++] = '0' + digit_val;
	    }
	
	    // Write the exponent
	    if (exp != 0) {
	    	str[pos++] = 'e';
	    	utils::int_to_char_array(str, pos, exp);
	    }
	}
	

    template<char fmt, size_t N, typename T>
        requires float_format<fmt> && (!std::is_floating_point_v<T>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
        static_assert(fmt == '\0', "'%f' expected an floating point type");
    }

    
    // 
    // %s -> string-like objects
    //
    
    
    template<char fmt>
    concept string_format = fmt == 's';
    
    
    // char array
    
    
    template<char fmt, typename T>
    	requires string_format<fmt> && (utils::is_char_array_holder<T> || utils::is_dyn_str_holder<T>)
    consteval size_t formatted_str_length()
    {
    	if constexpr (T::size() == 0) {
    		return 0;
    	}
    	else {
    		return T::size();
    	}
    }
    
    
    template<char fmt, size_t N, typename T>
    	requires string_format<fmt> && (utils::is_char_array_holder<T> || utils::is_dyn_str_holder<T>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
        if constexpr (T::size() > 0) {
            constexpr size_t STR_LEN = T::size();
            for (size_t i = 0; i < STR_LEN; i++) {
                char c;
                if constexpr (utils::is_char_array_holder<T>) {
                	c = T::get()[i];
                }
                else {
                	c = val.str[i];
                }
                if (c == '\0') { break; }
                str[pos++] = c;
            }
        }
    }
    
    
    // string_view
    
    
    template<char fmt, typename T>
    	requires string_format<fmt> && utils::is_str_view_holder<T>
    consteval size_t formatted_str_length()
    {
    	return T::get().size();
    }
    
    
    template<char fmt, size_t N, typename T>
    	requires string_format<fmt> && utils::is_str_view_holder<T>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
    	constexpr size_t STR_LEN = T::get().size();
    	for (size_t i = 0; i < STR_LEN; i++) {
    		str[pos + i] = T::get()[i];
    	}
    	pos += STR_LEN;
    }
    
    
    // Wrong string argument
    
    
    template<char fmt, typename T>
    	requires string_format<fmt>
                 && (!utils::is_char_array_holder<T>)
                 && (!utils::is_str_view_holder<T>)
                 && (!utils::is_dyn_str_holder<T>)
    consteval size_t formatted_str_length()
    {
        static_assert(fmt == '\0', "'%s' expected a string view (or char array) holder");
        return 0;
    }


    template<char fmt, size_t N, typename T>
        requires string_format<fmt>
             && (!utils::is_char_array_holder<T>)
             && (!utils::is_str_view_holder<T>)
             && (!utils::is_dyn_str_holder<T>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
        static_assert(fmt == '\0', "'%s' expected a string view (or char array) holder");
    }


    //
    // %c -> character
    //


    template<char fmt>
    concept char_format = fmt == 'c';


    template<char fmt, typename T>
        requires char_format<fmt> && std::same_as<std::remove_cv_t<T>, char>
    consteval size_t formatted_str_length()
    {
        return 1;
    }


    template<char fmt, typename T>
        requires char_format<fmt> && (!std::same_as<std::remove_cv_t<T>, char>)
    consteval size_t formatted_str_length()
    {
        static_assert(fmt == '\0', "'%c' expected a char type");
        return 0;
    }


    template<char fmt, size_t N, typename T>
        requires char_format<fmt> && std::same_as<std::remove_cv_t<T>, char>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
        str[pos++] = val;
    }


    template<char fmt, size_t N, typename T>
        requires char_format<fmt> && (!std::same_as<std::remove_cv_t<T>, char>)
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
        static_assert(fmt == '\0', "'%c' expected a char type");
    }

    
    /**
     * Fallback option, used only when 'fmt' is not specified by another definition.
     * Made to fail in all cases.
     */
    template<char fmt, typename, typename... Args>
    [[maybe_unused]]
    consteval size_t formatted_str_length(const Args&...)
    {
        static_assert(fmt == '\0', "Unknown format specifier");
        static_assert(fmt != '\0', "'\\0' is not a valid format specifier");
        return 0;
    }

    
    /**
     * Fallback option, used only when 'fmt' is not specified by another definition.
     * Made to fail in all cases.
     */
    template<char fmt>
    [[maybe_unused]]
    constexpr void format_to_str(...)
    {
        static_assert(fmt == '\0', "Unknown format specifier");
        static_assert(fmt != '\0', "'\\0' is not a valid format specifier");
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
               + specialisation::formatted_str_length<fmt[nxt], std::remove_reference_t<T>>() // Maximum length of the formatted type
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
    constexpr void parse_format_internal(std::array<char, N>& str, size_t& str_pos, const T& val, Args&&... args)
    {
        constexpr size_t nxt = next_format<fmt, pos>();

        // Copy the characters between two format positions to the string, excluding the '%'
        copy_fmt_to_array<fmt, N, pos, nxt - 1>(str, str_pos);

        // Format the value
        specialisation::format_to_str<fmt[nxt]>(str, str_pos, val);

        parse_format_internal<fmt, N, nxt + 1>(str, str_pos, std::forward<Args>(args)...);
    }
}


namespace cst_fmt
{
	/**
	 * Static char array reference holder.
	 * The copy will stop before the first '\0' character encountered.
	 */
	template<size_t N, const char (&STR)[N]>
	using cstr_ref = utils::CharArrayHolder<N, STR>;
	
	
	/**
	 * Non-const string with static fixed size reference holder.
	 * Up to N characters will be copied to the resulting string.
	 * The copy will stop before the first '\0' character encountered.
	 */
	template<size_t N>
	using cstr = utils::DynStrHolder<N>;

	
	/**
	 * Static string view reference holder.
	 * All characters (even '\0') of the string view will be copied.
	 */
	template<const std::string_view& STR>
	using str_ref = utils::StrViewHolder<STR>;


    /**
     * Parses the given format string and returns information reusable for calls to 'cst_fmt::format'.
     */
    template<const std::string_view& fmt, typename... Args>
    consteval auto compile_format()
    {
        constexpr size_t str_size = internal::get_formatted_str_length_start<fmt, Args...>();
        return CompiledFormat<fmt, str_size>{};
    }


    /**
     * Formats the given arguments by using the information returned by 'cst_fmt::format'.
     */
    template<const std::string_view& fmt, size_t str_size, typename... Args>
    constexpr auto format([[maybe_unused]] CompiledFormat<fmt, str_size> compiled_format, Args&&... args)
    {
        FormattedCharArray<str_size> str{};

        size_t str_pos = 0;
        internal::parse_format_internal<fmt, str_size, 0>(str, str_pos, std::forward<Args>(args)...);
        str.set_effective_size(str_pos);

        return str;
    }


    /**
     * Formats the arguments into a FormattedCharArray, which is a std::array<char, N> which length is determined solely
     * on the format string. This result can be converted to a string_view, string or const char*.
     * The format must be a static constexpr string_view.
     */
    template<const std::string_view& fmt, typename... Args>
    constexpr auto format(Args&&... args)
    {
        constexpr size_t str_size = internal::get_formatted_str_length_start<fmt, Args...>();
        FormattedCharArray<str_size> str{};

        size_t str_pos = 0;
        internal::parse_format_internal<fmt, str_size, 0>(str, str_pos, std::forward<Args>(args)...);
        str.set_effective_size(str_pos);

        return str;
    }
}


#endif //CONSTEXPRFORMAT_CONST_FORMAT_H
