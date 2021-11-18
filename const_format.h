﻿
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
        static constexpr const std::string_view& get_fmt() { return fmt; }

        [[nodiscard]]
        static constexpr size_t get_str_size() { return N; }
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
    
    
    template<char fmt>
    concept StringFormat = fmt == 's';
    
    
    // char array
    
    
    template<char fmt, typename T>
    	requires StringFormat<fmt> && utils::is_char_array_holder<T>
    consteval size_t formatted_str_length()
    {
    	if constexpr (T::size() == 0) {
    		return 0;
    	}
    	else {
    		return T::size() - 1;
    	}
    }
    
    
    template<char fmt, size_t N, typename T>
    	requires StringFormat<fmt> && utils::is_char_array_holder<T>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
    	if constexpr (T::size() > 1) {
    		constexpr size_t STR_LEN = T::size() - 1;
    		for (size_t i = 0; i < STR_LEN; i++) {
    			str[pos + i] = T::get()[i];
    		}
    	}
    }
    
    
    // string_view
    
    
    template<char fmt, typename T>
    	requires StringFormat<fmt> && utils::is_str_view_holder<T>
    consteval size_t formatted_str_length()
    {
    	return T::get().size();
    }
    
    
    template<char fmt, size_t N, typename T>
    	requires StringFormat<fmt> && utils::is_str_view_holder<T>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T&)
    {
    	constexpr size_t STR_LEN = T::get().size();
    	for (size_t i = 0; i < STR_LEN; i++) {
    		str[pos + i] = T::get()[i];
    	}
    	pos += STR_LEN;
    }
    
    
    // Non-const string
    
    
    template<char fmt, typename T>
    	requires StringFormat<fmt> && utils::is_dyn_str_holder<T>
    consteval size_t formatted_str_length()
    {
    	if constexpr (T::size() == 0) {
    		return 0;
    	}
    	else {
    		return T::size() - 1;
    	}
    }
    
    
    template<char fmt, size_t N, typename T>
    	requires StringFormat<fmt> && utils::is_dyn_str_holder<T>
    constexpr void format_to_str(std::array<char, N>& str, size_t& pos, const T& val)
    {
        if constexpr (T::size() > 0) {
            constexpr size_t STR_LEN = T::size();
            for (size_t i = 0; i < STR_LEN; i++) {
                char c = val.str[i];
                if (c == '\0') { break; }
                str[pos++] = c;
            }
        }
    }
    
    
    template<char fmt, typename T>
    	requires StringFormat<fmt> 
    		&& (!utils::is_char_array_holder<T>)
    		&& (!utils::is_str_view_holder<T>)
    		&& (!utils::is_dyn_str_holder<T>)
    consteval size_t formatted_str_length()
    {
        static_assert(fmt == '\0', "'%s' expected a string view (or char array) holder");
        return 0;
    }
    
    
    /**
     * Default option, used only when 'fmt' is not specified by another definition.
     * Made to fail in all cases.
     */
    template<char fmt, typename, typename... Args>
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
	/**
	 * Static char array reference holder.
	 * It is assumed that the last character of the array (at N-1) is '\0', and will not be copied.
	 */
	template<size_t N, const char (&STR)[N]>
	using cstr_ref = utils::CharArrayHolder<N, STR>;


	/**
	 * Static string view reference holder.
	 * All characters (even '\0') of the string view will be copied.
	 */
	template<const std::string_view& STR>
	using str_ref = utils::StrViewHolder<STR>;
	
	
	/**
	 * Non-const string with static fixed size reference holder.
	 * Up to N characters will be copied to the resulting string. The copy will stop before the first '\0' character
	 * encountered.
	 */
	template<size_t N>
	using dyn_str = utils::DynStrHolder<N>;
	
	
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
        // TODO : std::forward(args)... and use only r-values in parameters
        internal::parse_format_internal<fmt, str_size, 0>(str, str_pos, args...);
        str.set_effective_size(str_pos);

        return str;
    }
}


#endif //CONSTEXPRFORMAT_CONST_FORMAT_H
