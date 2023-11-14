// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_PARSER_HPP
#define BOOST_DECIMAL_DETAIL_PARSER_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/from_chars_result.hpp>
#include <boost/decimal/detail/from_chars_integer_impl.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <type_traits>
#include <limits>
#include <cerrno>
#include <cstdint>
#include <cstring>

namespace boost {
namespace decimal {
namespace detail {

constexpr auto is_integer_char(char c) noexcept -> bool
{
    return (c >= '0') && (c <= '9');
}

constexpr auto is_delimiter(char c) noexcept -> bool
{
    return !is_integer_char(c) && c != 'e' && c != 'E';
}

constexpr auto from_chars_dispatch(const char* first, const char* last, std::uint64_t& value, int base) noexcept -> from_chars_result
{
    return boost::decimal::detail::from_chars(first, last, value, base);
}

constexpr auto from_chars_dispatch(const char* first, const char* last, uint128& value, int base) noexcept -> from_chars_result
{
    return boost::decimal::detail::from_chars128(first, last, value, base);
}

#ifdef BOOST_CHARCONV_HAS_INT128
auto from_chars_dispatch(const char* first, const char* last, boost::uint128_type& value, int base) noexcept -> from_chars_result
{
    return boost::decimal::detail::from_chars128(first, last, value, base);
}
#endif

template <typename Unsigned_Integer, typename Integer>
constexpr auto parser(const char* first, const char* last, bool& sign, Unsigned_Integer& significand, Integer& exponent) noexcept -> from_chars_result
{
    if (first >= last)
    {
        return {first, std::errc::invalid_argument};
    }

    auto next = first;
    bool all_zeros = true;

    // First extract the sign
    if (*next == '-')
    {
        sign = true;
        ++next;
    }
    else if (*next == '+')
    {
        return {next, std::errc::invalid_argument};
    }
    else
    {
        sign = false;
    }

    // Handle non-finite values
    if (*next == 'i' || *next == 'I')
    {
        significand = 0;
        exponent = 0;
        return {next, std::errc::value_too_large};
    }
    if (*next == 'n' || *next == 'N')
    {
        const auto dist = last - next;

        if (dist > 4 && *(next + 4) == 's')
        {
            significand = 1;
            exponent = 0;
            return {next, std::errc::not_supported};
        }
        else
        {
            significand = 0;
            exponent = 0;
            return {next, std::errc::not_supported};
        }
    }


    // Ignore leading zeros (e.g. 00005 or -002.3e+5)
    while (*next == '0' && next != last)
    {
        ++next;
    }

    // If the number is 0 we can abort now
    constexpr char exp_char {'e'};
    constexpr char capital_exp_char {'E'};

    if (next == last || *next == exp_char || *next == -capital_exp_char)
    {
        if (next == first)
        {
            return {first, std::errc::invalid_argument};
        }
        
        significand = 0;
        exponent = 0;
        return {next, std::errc()};
    }

    // Next we get the significand
    constexpr std::size_t significand_buffer_size = std::numeric_limits<Unsigned_Integer>::digits10 ;
    char significand_buffer[significand_buffer_size] {};
    std::size_t i = 0;
    std::size_t dot_position = 0;
    Integer extra_zeros = 0;
    Integer leading_zero_powers = 0;
    const auto char_validation_func = is_integer_char;
    const int base = 10;

    while (char_validation_func(*next) && next != last && i < significand_buffer_size)
    {
        all_zeros = false;
        significand_buffer[i] = *next;
        ++next;
        ++i;
    }

    bool fractional = false;
    if (next == last)
    {
        exponent = 0;
        std::size_t offset = i;

        from_chars_result r = from_chars_dispatch(significand_buffer, significand_buffer + offset, significand, base);

        return {next, r.ec};
    }
    else if (*next == '.')
    {
        ++next;
        fractional = true;
        dot_position = i;

        // Process the fractional part if we have it
        //
        // if fmt is chars_format::scientific the e is required
        // if fmt is chars_format::fixed and not scientific the e is disallowed
        // if fmt is chars_format::general (which is scientific and fixed) the e is optional

        // If we have the value 0.00001 we can continue to chop zeros and adjust the exponent
        // so that we get the useful parts of the fraction
        if (all_zeros)
        {
            while (*next == '0' && next != last)
            {
                ++next;
                --leading_zero_powers;
            }

            if (next == last)
            {
                return {last, std::errc()};
            }
        }

        while (char_validation_func(*next) && next != last && i < significand_buffer_size)
        {
            significand_buffer[i] = *next;
            ++next;
            ++i;
        }
    }
    
    if (i == significand_buffer_size)
    {
        // We can not process any more significant figures into the significand so skip to the end
        // or the exponent part and capture the additional orders of magnitude for the exponent
        bool found_dot = false;
        while ((char_validation_func(*next) || *next == '.') && next != last)
        {
            ++next;
            if (!fractional && !found_dot)
            {
                ++extra_zeros;
            }
            if (*next == '.')
            {
                found_dot = true;
            }
        }
    }

    if (next == last || is_delimiter(*next))
    {
        if (dot_position != 0 || fractional)
        {
            exponent = static_cast<Integer>(dot_position - i) + extra_zeros + leading_zero_powers;
        }
        else
        {
            exponent = extra_zeros + leading_zero_powers;
        }
        std::size_t offset = i;
        
        from_chars_result r = from_chars_dispatch(significand_buffer, significand_buffer + offset, significand, base);
        switch (r.ec)
        {
            case std::errc::invalid_argument:
                return {first, std::errc::invalid_argument};
            case std::errc::result_out_of_range:
                return {next, std::errc::result_out_of_range};
            default:
                return {next, std::errc()};
        }
    }
    else if (*next == exp_char || *next == capital_exp_char)
    {
        ++next;

        exponent = static_cast<Integer>(i - 1);
        std::size_t offset = i;
        bool round = false;
        // If more digits are present than representable in the significand of the target type
        // we set the maximum
        if (offset == significand_buffer_size)
        {
            offset = significand_buffer_size - 1;
            i = significand_buffer_size;
            if (significand_buffer[offset] == '5' ||
                significand_buffer[offset] == '6' ||
                significand_buffer[offset] == '7' ||
                significand_buffer[offset] == '8' ||
                significand_buffer[offset] == '9')
            {
                round = true;
            }
        }
        
        // If the significand is 0 from chars will return std::errc::invalid_argument because there is nothing in the buffer,
        // but it is a valid value. We need to continue parsing to get the correct value of ptr even
        // though we know we could bail now.
        //
        // See GitHub issue #29: https://github.com/cppalliance/charconv/issues/29
        if (offset != 0)
        {
            BOOST_DECIMAL_ATTRIBUTE_UNUSED from_chars_result r = from_chars_dispatch(significand_buffer, significand_buffer + offset, significand, base);

            static_cast<void>(r);

            if (round)
            {
                significand += 1;
            }
        }
    }
    else
    {
        return {first, std::errc::invalid_argument};
    }

    // Finally we get the exponent
    constexpr std::size_t exponent_buffer_size = 6; // Float128 min exp is −16382
    char exponent_buffer[exponent_buffer_size] {};
    const auto significand_digits = i;
    i = 0;

    // Get the sign first
    if (*next == '-')
    {
        exponent_buffer[i] = *next;
        ++next;
        ++i;
    }
    else if (*next == '+')
    {
        ++next;
    }

    // Next strip any leading zeros
    while (*next == '0')
    {
        ++next;
    }

    // Process the significant values
    while (is_integer_char(*next) && next != last && i < exponent_buffer_size)
    {
        exponent_buffer[i] = *next;
        ++next;
        ++i;
    }

    // If the exponent can't fit in the buffer the number is not representable
    if (next != last && i == exponent_buffer_size)
    {
        return {next, std::errc::result_out_of_range};
    }

    // If the exponent was e+00 or e-00
    if (i == 0 || (i == 1 && exponent_buffer[0] == '-'))
    {
        if (fractional)
        {
            exponent = static_cast<Integer>(dot_position - significand_digits);
        }
        else
        {
            exponent = extra_zeros;
        }

        return {next, std::errc()};
    }

    const auto r = from_chars(exponent_buffer, exponent_buffer + i, exponent);

    exponent += leading_zero_powers;

    if (fractional)
    {
        // Need to take the offset from 1.xxx because compute_floatXXX assumes the significand is an integer
        // so the exponent is off by the number of digits in the significand - 1
        exponent -= static_cast<Integer>(significand_digits - dot_position);
    }
    else
    {
        exponent += extra_zeros;
    }

    return {next, r.ec};
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_PARSER_HPP
