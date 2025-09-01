// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CHARCONV_HPP
#define BOOST_DECIMAL_CHARCONV_HPP

#include <boost/decimal/decimal32_t.hpp>
#include <boost/decimal/decimal64_t.hpp>
#include <boost/decimal/decimal128_t.hpp>
#include <boost/decimal/decimal_fast32_t.hpp>
#include <boost/decimal/decimal_fast64_t.hpp>
#include <boost/decimal/decimal_fast128_t.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include "detail/int128.hpp"
#include <boost/decimal/detail/from_chars_result.hpp>
#include <boost/decimal/detail/chars_format.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/to_chars_result.hpp>
#include <boost/decimal/detail/to_chars_integer_impl.hpp>
#include <boost/decimal/detail/buffer_sizing.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/countl.hpp>
#include <boost/decimal/detail/remove_trailing_zeros.hpp>
#include <boost/decimal/detail/promotion.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <string>
#endif

namespace boost {
namespace decimal {

// ---------------------------------------------------------------------------------------------------------------------
// from_chars and implementation
// ---------------------------------------------------------------------------------------------------------------------

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto from_chars_general_impl(const char* first, const char* last, TargetDecimalType& value, const chars_format fmt) noexcept -> from_chars_result
{
    using significand_type = std::conditional_t<(std::numeric_limits<typename TargetDecimalType::significand_type>::digits >
                                                 std::numeric_limits<std::uint64_t>::digits),
                                                 int128::uint128_t, std::uint64_t>;

    if (first >= last)
    {
        return {first, std::errc::invalid_argument};
    }

    bool sign {};
    significand_type significand {};
    std::int32_t expval {};

    auto r {detail::parser(first, last, sign, significand, expval, fmt)};

    if (!r)
    {
        if (r.ec == std::errc::not_supported)
        {
            if (significand)
            {
                value = std::numeric_limits<TargetDecimalType>::signaling_NaN();
            }
            else
            {
                value = std::numeric_limits<TargetDecimalType>::quiet_NaN();
            }

            r.ec = std::errc();
        }
        else if (r.ec == std::errc::value_too_large)
        {
            value = std::numeric_limits<TargetDecimalType>::infinity();
            r.ec = std::errc::result_out_of_range;
        }
        else
        {
            value = std::numeric_limits<TargetDecimalType>::signaling_NaN();
            errno = static_cast<int>(r.ec);
        }
    }
    else
    {
        value = TargetDecimalType(significand, expval, sign);
    }

    return r;
}

} //namespace detail

BOOST_DECIMAL_EXPORT template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto from_chars(const char* first, const char* last, TargetDecimalType& value, const chars_format fmt = chars_format::general) noexcept -> from_chars_result
{
    return detail::from_chars_general_impl(first, last, value, fmt);
}

#ifndef BOOST_DECIMAL_HAS_STD_STRING_VIEW

BOOST_DECIMAL_EXPORT template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto from_chars(const std::string& str, TargetDecimalType& value, const chars_format fmt = chars_format::general) noexcept -> from_chars_result
{
    return detail::from_chars_general_impl(str.data(), str.data() + str.size(), value, fmt);
}

#else

BOOST_DECIMAL_EXPORT template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto from_chars(std::string_view str, TargetDecimalType& value, chars_format fmt = chars_format::general) noexcept -> from_chars_result
{
    return detail::from_chars_general_impl(str.data(), str.data() + str.size(), value, fmt);
}

#endif

#ifdef BOOST_DECIMAL_HAS_STD_CHARCONV
BOOST_DECIMAL_EXPORT template <typename DecimalType>
constexpr auto from_chars(const char* first, const char* last, DecimalType& value, std::chars_format fmt) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::from_chars_result)
{
    from_chars_result boost_r {};
    switch (fmt)
    {
        case std::chars_format::scientific:
            boost_r = from_chars(first, last, value, chars_format::scientific);
            break;
        case std::chars_format::fixed:
            boost_r = from_chars(first, last, value, chars_format::fixed);
            break;
        case std::chars_format::hex:
            boost_r = from_chars(first, last, value, chars_format::hex);
            break;
        case std::chars_format::general:
            boost_r = from_chars(first, last, value, chars_format::general);
            break;
        // LCOV_EXCL_START
        default:
            BOOST_DECIMAL_UNREACHABLE;
        // LCOV_EXCL_STOP
    }

    return std::from_chars_result {boost_r.ptr, boost_r.ec};
}

BOOST_DECIMAL_EXPORT template <typename DecimalType>
constexpr auto from_chars(std::string_view str, DecimalType& value, std::chars_format fmt) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::from_chars_result)
{
    return from_chars(str.data(), str.data() + str.size(), value, fmt);
}
#endif

// ---------------------------------------------------------------------------------------------------------------------
// to_chars and implementation
// ---------------------------------------------------------------------------------------------------------------------

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars_nonfinite(char* first, char* last, const TargetDecimalType& value, const int fp, const chars_format fmt, const int local_precision) noexcept -> to_chars_result
{
    const auto buffer_len = last - first;

    switch (fp)
    {
        case FP_INFINITE:
            if (buffer_len >= 3)
            {
                boost::decimal::detail::memcpy(first, "inf", 3U);
                return {first + 3U, std::errc()};
            }

            return {last, std::errc::value_too_large};
        case FP_ZERO:
            if (fmt == chars_format::general)
            {
                *first++ = '0';
                return {first, std::errc()};
            }
            else if (fmt == chars_format::hex || fmt == chars_format::scientific)
            {
                if (buffer_len >= 7 + local_precision + 1)
                {
                    if (local_precision <= 0)
                    {
                        *first++ = '0';
                    }
                    else
                    {
                        boost::decimal::detail::memcpy(first, "0.0", 3U);
                        first += 3U;

                        if (local_precision != 1)
                        {
                            boost::decimal::detail::memset(first, '0', static_cast<std::size_t>(local_precision - 1));
                            first += local_precision - 1;
                        }
                    }

                    if (fmt == chars_format::hex)
                    {
                        *first++ = 'p';
                    }
                    else
                    {
                        *first++ = 'e';
                    }

                    boost::decimal::detail::memcpy(first, "+00", 3U);
                    return {first + 3U, std::errc()};
                }
            }
            else
            {
                if (local_precision == -1 || local_precision == 0)
                {
                    *first++ = '0';
                    return {first, std::errc()};
                }
                else if (buffer_len > 2 + local_precision)
                {
                    boost::decimal::detail::memcpy(first, "0.0", 3U);
                    first += 3U;

                    if (local_precision > 1)
                    {
                        boost::decimal::detail::memset(first, '0', static_cast<std::size_t>(local_precision - 1));
                        first += local_precision - 1;
                    }

                    return {first, std::errc()};
                }
            }

            return {last, std::errc::value_too_large};
        case FP_NAN:
            if (issignaling(value) && buffer_len >= 9)
            {
                boost::decimal::detail::memcpy(first, "nan(snan)", 9U);
                return {first + 9U, std::errc()};
            }
            else if (signbit(value) && buffer_len >= 9)
            {
                boost::decimal::detail::memcpy(first, "nan(ind)", 8U);
                return {first + 8U, std::errc()};
            }
            else if (buffer_len >= 3)
            {
                boost::decimal::detail::memcpy(first, "nan", 3U);
                return {first + 3U, std::errc()};
            }

            return {last, std::errc::value_too_large};
        // LCOV_EXCL_START
        default:
            BOOST_DECIMAL_UNREACHABLE;
        // LCOV_EXCL_STOP
    }
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto to_chars_scientific_impl(char* first, char* last, const TargetDecimalType& value, const chars_format fmt) noexcept -> to_chars_result
{
    bool is_neg {false};
    if (signbit(value))
    {
        *first++ = '-';
        is_neg = true;
    }

    const auto fp = fpclassify(value);
    if (!(fp == FP_NORMAL || fp == FP_SUBNORMAL))
    {
        return to_chars_nonfinite(first, last, value, fp, fmt, -1);
    }

    const auto buffer_size {last - first};
    const auto real_precision {get_real_precision<TargetDecimalType>()};

    // Dummy check the bounds
    if (buffer_size < real_precision)
    {
        return {last, std::errc::value_too_large};
    }

    using uint_type = std::conditional_t<(std::numeric_limits<typename TargetDecimalType::significand_type>::digits >
                                              std::numeric_limits<std::uint64_t>::digits),
                                              int128::uint128_t, std::uint64_t>;

    // Need to offset the exp for the fact that it's not 123e+2, it's 1.23e+4
    const auto components {value.to_components()};
    auto r = to_chars_integer_impl(first + 1, last, static_cast<uint_type>(components.sig));

    // Only real reason we will hit this is a buffer overflow,
    // which we have already checked for
    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    const auto num_digits {r.ptr - (first + 1)};

    // Any trailing zeros can be removed
    // This is faster than stripping them from the normalized number
    --r.ptr;
    while (*r.ptr == '0')
    {
        --r.ptr;
    }
    ++r.ptr;

    auto exp {components.exp + num_digits - 1};

    // Make sure the result will fit in the buffer before continuing progress
    const auto total_length {total_buffer_length<TargetDecimalType>(static_cast<int>(num_digits), exp, is_neg)};
    if (total_length > buffer_size)
    {
        return {last, std::errc::value_too_large};
    }

    // Insert our decimal point
    *first = *(first + 1);
    *(first + 1) = '.';
    first = r.ptr;

    *first++ = 'e';
    if (exp >= 0)
    {
        *first++ = '+';
    }
    else
    {
        *first++ = '-';
        exp = -exp;
    }

    // Need at least two digits e.g. e-09
    if (exp < 10)
    {
        *first++ = '0';
    }

    const auto exp_r {to_chars_integer_impl(first, last, exp)};

    if (BOOST_DECIMAL_UNLIKELY(!exp_r))
    {
        return exp_r; // LCOV_EXCL_LINE
    }

    return {exp_r.ptr, std::errc{}};
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars_scientific_impl(char* first, char* last, const TargetDecimalType& value, const chars_format fmt, const int local_precision) noexcept -> to_chars_result
{
    if (signbit(value))
    {
        *first++ = '-';
    }

    const auto fp = fpclassify(value);
    if (!(fp == FP_NORMAL || fp == FP_SUBNORMAL))
    {
        return to_chars_nonfinite(first, last, value, fp, fmt, local_precision);
    }

    int exp {};
    auto significand {frexp10(value, &exp)};

    using uint_type = std::conditional_t<(std::numeric_limits<typename TargetDecimalType::significand_type>::digits >
                                          std::numeric_limits<std::uint64_t>::digits),
                                          int128::uint128_t, std::uint64_t>;

    // Since frexp10 normalizes the value, we by default know the number of digits in the significand
    auto significand_digits = std::numeric_limits<TargetDecimalType>::digits;
    exp += significand_digits - 1;
    bool append_zeros = false;

    if (local_precision != -1)
    {
        if (significand_digits > local_precision)
        {
            // If the precision is specified, we need to make sure the result is rounded correctly
            // using the current fenv rounding mode

            if (significand_digits > local_precision + 2)
            {
                const auto digits_to_remove {significand_digits - (local_precision + 2)};
                significand /= pow10(static_cast<typename TargetDecimalType::significand_type>(digits_to_remove));
                significand_digits -= digits_to_remove;
                const auto original_sig {significand};
                fenv_round(significand);
                if (remove_trailing_zeros(original_sig + 1U).trimmed_number == 1U)
                {
                    ++exp;
                    if (exp == 0)
                    {
                        *first++ = '1';
                        if (local_precision > 0)
                        {
                            *first++ = '.';
                            detail::memset(first, '0', static_cast<std::size_t>(local_precision));
                            first += local_precision;
                        }
                        detail::memcpy(first, "e+00", 4u);
                        return {first + 4u, std::errc()};
                    }
                }
            }
            else if (significand_digits > local_precision + 1)
            {
                const auto original_sig = significand;
                fenv_round(significand);
                if (remove_trailing_zeros(original_sig + 1U).trimmed_number == 1U)
                {
                    ++exp;
                    if (exp == 0)
                    {
                        *first++ = '1';
                        if (local_precision > 0)
                        {
                            *first++ = '.';
                            detail::memset(first, '0', static_cast<std::size_t>(local_precision));
                            first += local_precision;
                        }
                        detail::memcpy(first, "e+00", 4u);
                        return {first + 4u, std::errc()};
                    }
                }
            }
        }
        else if (significand_digits < local_precision && fmt != chars_format::general)
        {
            append_zeros = true;
        }
    }

    // Offset the value of first by 1 so that we can copy the leading digit and insert a decimal point
    auto r = to_chars_integer_impl<uint_type>(first + 1, last, significand);

    // Only real reason we will hit this is a buffer overflow
    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    const auto current_digits = r.ptr - (first + 1) - 1;

    if (current_digits < local_precision && fmt != chars_format::general)
    {
        append_zeros = true;
    }

    if (append_zeros)
    {
        const auto zeros_inserted {static_cast<std::size_t>(local_precision - current_digits)};

        if (r.ptr + zeros_inserted > last)
        {
            return {last, std::errc::value_too_large};
        }

        boost::decimal::detail::memset(r.ptr, '0', zeros_inserted);
        r.ptr += zeros_inserted;
    }

    // Insert our decimal point
    *first = *(first + 1);
    *(first + 1) = '.';
    first = r.ptr;

    if (local_precision == 0)
    {
        --first;
    }

    // Strip trailing zeros in general mode
    if (fmt == chars_format::general)
    {
        --first;
        while (*first == '0')
        {
            --first;
        }

        // Remove decimal point if not significant digits
        if (*first != '.')
        {
            ++first;
        }
    }

    // Insert the exponent character
    *first++ = 'e';

    const int abs_exp { (exp < 0) ? -exp : exp };

    if (exp < 0)
    {
        *first++ = '-';
    }
    else
    {
        *first++ = '+';
    }

    // Always give 2 digits in the exp (ex. 2.0e+09)
    if (abs_exp <= 9)
    {
        *first++ = '0';
    }

    r = to_chars_integer_impl<int>(first, last, abs_exp);
    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    return {r.ptr, std::errc()};
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto to_chars_fixed_impl(char* first, char* last, const TargetDecimalType& value, const chars_format fmt) noexcept -> to_chars_result
{
    const auto buffer_size {last - first};
    const auto real_precision {get_real_precision<TargetDecimalType>()};

    // Dummy check the bounds
    if (buffer_size < real_precision)
    {
        return {last, std::errc::value_too_large};
    }

    bool is_neg {false};
    if (signbit(value))
    {
        *first++ = '-';
        is_neg = true;
    }

    const auto fp = fpclassify(value);
    if (!(fp == FP_NORMAL || fp == FP_SUBNORMAL))
    {
        return to_chars_nonfinite(first, last, value, fp, fmt, -1);
    }

    const auto components {value.to_components()};

    const auto r {to_chars_integer_impl(first, last, components.sig)};

    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    // We now have the complete number written into the buffer
    // The question now becomes where does the decimal point go
    const auto num_digits {r.ptr - (first + 1)};
    const auto exp {components.exp};
    const auto abs_exp {exp < 0 ? -exp : exp};

    // There are now three cases that we need to handle
    // 1) We need to append trailing zeros e.g. 12345000000
    // 2) We need to insert the decimal point 12.345
    // 3) We need to append leading zeros e.g 0.0000012345

    if (exp >= 0)
    {
        if (buffer_size < num_digits + exp + static_cast<int>(is_neg))
        {
            return {last, std::errc::value_too_large};
        }

        detail::memset(r.ptr, '0', static_cast<std::size_t>(exp));
        return {r.ptr + exp, std::errc{}};
    }
    else if (exp < 0 && abs_exp > num_digits)
    {
        if (buffer_size < num_digits + 1 + static_cast<int>(is_neg))
        {
            return {last, std::errc::value_too_large};
        }

        detail::memmove(r.ptr + exp + 1, r.ptr + exp, static_cast<std::size_t>(abs_exp));
        detail::memset(r.ptr + exp, '.', 1U);

        return {r.ptr + 1U, std::errc{}};
    }
    else
    {
        if (buffer_size < num_digits + abs_exp + 1 + static_cast<int>(is_neg))
        {
            return {last, std::errc::value_too_large};
        }

        detail::memmove(r.ptr + abs_exp + 2, first, static_cast<std::size_t>(num_digits));
        *first++ = '0';
        *first++ = '.';
        detail::memset(first, '0', static_cast<std::size_t>(abs_exp));

        return {r.ptr + abs_exp + 2 + num_digits, std::errc{}};
    }

    BOOST_DECIMAL_UNREACHABLE; // LCOV_EXCL_LINE
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars_fixed_impl(char* first, char* last, const TargetDecimalType& value, const chars_format fmt, const int local_precision) noexcept -> to_chars_result
{
    using target_decimal_significand_type = typename TargetDecimalType::significand_type;

    auto buffer_size = last - first;
    auto real_precision = get_real_precision<TargetDecimalType>(local_precision);

    // Dummy check the bounds
    if (buffer_size < real_precision)
    {
        return {last, std::errc::value_too_large};
    }

    const bool is_neg = signbit(value);
    if (is_neg)
    {
        *first++ = '-';
        --buffer_size;
    }

    const auto fp = fpclassify(value);
    if (!(fp == FP_NORMAL || fp == FP_SUBNORMAL))
    {
        return to_chars_nonfinite(first, last, value, fp, fmt, local_precision);
    }

    auto abs_value = abs(value);
    int exponent {};
    target_decimal_significand_type significand = frexp10(abs_value, &exponent);

    const char* output_start = first;

    int num_dig = std::numeric_limits<TargetDecimalType>::digits;
    bool append_trailing_zeros = false;
    bool append_leading_zeros = false;
    int num_leading_zeros = 0;
    int integer_digits = num_dig + exponent;
    num_dig -= integer_digits;

    if (integer_digits < 0)
    {
        const int abs_integer_digits { (integer_digits < 0) ? -integer_digits : integer_digits };

        num_leading_zeros = abs_integer_digits;
        integer_digits = 0;
        append_leading_zeros = true;
    }

    if (local_precision != -1)
    {
        if (num_dig > local_precision + 1)
        {
            const auto digits_to_remove {num_dig - local_precision - 1};
            if (digits_to_remove < std::numeric_limits<target_decimal_significand_type>::digits10 + 1)
            {
                significand /= pow10(static_cast<target_decimal_significand_type>(digits_to_remove));
                exponent += digits_to_remove + fenv_round<TargetDecimalType>(significand);
                num_dig -= digits_to_remove - 1;
            }
            else
            {
                significand = 0;
                num_dig = 0;
                exponent -= digits_to_remove + (local_precision + 1);
            }
        }
        else if (num_dig == local_precision + 1)
        {
            --num_dig;
            exponent += fenv_round<TargetDecimalType>(significand);
        }
        else if (num_dig < local_precision && fmt != chars_format::general)
        {
            append_trailing_zeros = true;
        }
    }

    // In general formatting, we remove trailing 0s
    // Same with unspecified precision fixed formatting
    if ((local_precision == -1 && fmt == chars_format::fixed) || fmt == chars_format::general)
    {
        const auto zeros_removal {remove_trailing_zeros(significand)};
        significand = zeros_removal.trimmed_number;
        exponent += static_cast<int>(zeros_removal.number_of_removed_zeros);
        num_dig -= static_cast<int>(zeros_removal.number_of_removed_zeros);
    }

    // We could have the case where we are rounding 0.9999 to 1.000
    if (-exponent >= 0 && -exponent < std::numeric_limits<target_decimal_significand_type>::digits10 &&
        significand == detail::pow10(static_cast<target_decimal_significand_type>(-exponent)) && fmt == chars_format::fixed)
    {
        *first++ = '1';
        if (local_precision > 0 && local_precision <= buffer_size)
        {
            *first++ = '.';
            detail::memset(first, '0', static_cast<std::size_t>(local_precision));
            return {first + local_precision, std::errc{}};
        }
        else if (local_precision > buffer_size)
        {
            return {last, std::errc::value_too_large};
        }
        else
        {
            return {first, std::errc{}};
        }
    }

    // Make sure the result will fit in the buffer
    const std::ptrdiff_t total_length = total_buffer_length<TargetDecimalType>(num_dig, exponent, is_neg) + num_leading_zeros;
    if (total_length > buffer_size)
    {
        return {last, std::errc::value_too_large};
    }

    // Insert the leading zeros and return if the answer is ~0 for current precision
    if (append_leading_zeros)
    {
        if (local_precision == 0)
        {
            *first++ = '0';
            return {first, std::errc()};
        }
        else if (local_precision != -1 && num_leading_zeros > local_precision)
        {
            *first++ = '0';
            *first++ = '.';
            boost::decimal::detail::memset(first, '0', static_cast<std::size_t>(local_precision));
            return {first + local_precision, std::errc()};
        }
        else
        {
            *first++ = '0';
            *first++ = '.';
            boost::decimal::detail::memset(first, '0', static_cast<std::size_t>(num_leading_zeros));
            first += num_leading_zeros;

            // We can skip the rest if there's nothing more to do for the required precision
            if (significand == 0U)
            {
                if (local_precision - num_leading_zeros > 0)
                {
                    boost::decimal::detail::memset(first, '0', static_cast<std::size_t>(local_precision - num_leading_zeros));
                    return {first + local_precision, std::errc()};
                }
                else
                {
                    return {first, std::errc()};
                }
            }
        }
    }

    using uint_type = std::conditional_t<(std::numeric_limits<typename TargetDecimalType::significand_type>::digits >
                                          std::numeric_limits<std::uint64_t>::digits),
                                          int128::uint128_t, std::uint64_t>;

    auto r = to_chars_integer_impl<uint_type>(first, last, significand);

    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    // Bounds check again
    if (local_precision == 0 && !append_trailing_zeros && !append_leading_zeros)
    {
        return {r.ptr, std::errc()};
    }
    else if (abs_value >= 1 || (significand == 1U && exponent == 0))
    {
        if (exponent < 0 && -exponent < buffer_size)
        {
            // Bounds check our move
            if (r.ptr + 2 > last)
            {
                return {last, std::errc::value_too_large};
            }

            boost::decimal::detail::memmove(r.ptr + exponent + 1, r.ptr + exponent,
                                            static_cast<std::size_t>(-exponent));
            boost::decimal::detail::memset(r.ptr + exponent, '.', 1U);
            ++r.ptr;
        }
        else if (exponent >= 1)
        {
            // Bounds check the length of the memset before doing so
            if (r.ptr + exponent + 1 > last)
            {
                return {last, std::errc::value_too_large};
            }

            boost::decimal::detail::memset(r.ptr, '0', static_cast<std::size_t>(exponent));
            r.ptr += exponent;

            if (append_trailing_zeros)
            {
                *r.ptr++ = '.';
            }
        }
        else if (append_trailing_zeros)
        {
            *r.ptr++ = '.';
        }
    }
    else if (!append_leading_zeros)
    {
        #ifdef BOOST_DECIMAL_DEBUG_FIXED
        std::cerr << std::setprecision(std::numeric_limits<Real>::digits10) << "Value: " << value
                  << "\n  Buf: " << first
                  << "\n  sig: " << significand
                  << "\n  exp: " << exponent << std::endl;
        #endif

        const auto offset_bytes = static_cast<std::size_t>(integer_digits);

        // Bounds check memmove followed by insertion of 0.
        if (first + 2 + offset_bytes + (static_cast<std::size_t>(-exponent) - offset_bytes) + 2 > last)
        {
            return {last, std::errc::value_too_large};
        }

        boost::decimal::detail::memmove(first + 2 + offset_bytes,
                                        first,
                                        static_cast<std::size_t>(-exponent) - offset_bytes);

        boost::decimal::detail::memcpy(first, "0.", 2U);
        first += 2;
        r.ptr += 2;
    }

    // The leading 0 is an integer digit now that we need to account for
    if (integer_digits == 0)
    {
        ++integer_digits;
    }

    const auto current_fractional_digits = r.ptr - output_start - integer_digits - 1;
    if (current_fractional_digits < local_precision && fmt != chars_format::general)
    {
        append_trailing_zeros = true;
    }

    if (append_trailing_zeros)
    {
        const auto zeros_inserted = static_cast<std::size_t>(local_precision - current_fractional_digits);

        if (r.ptr + zeros_inserted > last)
        {
            return {last, std::errc::value_too_large};
        }

        boost::decimal::detail::memset(r.ptr, '0', zeros_inserted);
        r.ptr += zeros_inserted;

        if (*(r.ptr - 1) == '.')
        {
            --r.ptr;
        }
    }

    return {r.ptr, std::errc()};
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars_hex_impl(char* first, char* last, const TargetDecimalType& value, const int local_precision = -1) noexcept -> to_chars_result
{
    using Unsigned_Integer = std::conditional_t<(std::numeric_limits<typename TargetDecimalType::significand_type>::digits >
                                                 std::numeric_limits<std::uint64_t>::digits),
                                                 int128::uint128_t, std::uint64_t>;

    if (signbit(value))
    {
        *first++ = '-';
    }

    const auto fp = fpclassify(value);
    if (!(fp == FP_NORMAL || fp == FP_SUBNORMAL))
    {
        return to_chars_nonfinite(first, last, value, fp, chars_format::hex, local_precision);
    }

    const std::ptrdiff_t buffer_size = last - first;
    auto real_precision = get_real_precision<TargetDecimalType>(precision);

    if (local_precision != -1)
    {
        real_precision = local_precision;
    }

    if (buffer_size < real_precision)
    {
        return {last, std::errc::value_too_large};
    }

    int exp {};
    Unsigned_Integer significand = frexp10(value, &exp);
    BOOST_DECIMAL_ASSERT(significand != 0U);
    // Strip zeros of the significand since frexp10 normalizes it
    const auto zero_removal {detail::remove_trailing_zeros(significand)};
    significand = zero_removal.trimmed_number;
    exp += static_cast<int>(zero_removal.number_of_removed_zeros);

    // Calculate the number of bytes
    constexpr auto significand_bits = std::is_same<Unsigned_Integer, std::uint64_t>::value ? 64 : 128;
    auto significand_digits = static_cast<int>(std::ceil(static_cast<double>(significand_bits - countl_zero(significand)) / 4));
    bool append_zeros = false;

    if (local_precision != -1)
    {
        if (significand_digits > local_precision + 2)
        {
            const auto shift_amount {significand_digits - (local_precision + 2)};
            significand >>= (shift_amount * 4);
            significand_digits -= shift_amount;
        }

        if (significand_digits > local_precision + 1)
        {
            const auto trailing_digit = static_cast<std::uint32_t>(significand & 0xFU);
            significand >>= 4;
            ++exp;
            if (trailing_digit >= 8)
            {
                ++significand;
            }
        }

        if (significand_digits < local_precision)
        {
            append_zeros = true;
        }
    }

    auto r = to_chars_integer_impl<Unsigned_Integer, Unsigned_Integer>(first + 1, last, significand, 16);
    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    const auto current_digits = r.ptr - (first + 1) - 1;
    exp += static_cast<int>(current_digits);

    if (current_digits < local_precision)
    {
        append_zeros = true;
    }

    if (append_zeros)
    {
        const auto zeros_inserted {static_cast<std::size_t>(local_precision - current_digits)};

        if (r.ptr + zeros_inserted > last)
        {
            return {last, std::errc::value_too_large};
        }

        boost::decimal::detail::memset(r.ptr, '0', zeros_inserted);
        r.ptr += zeros_inserted;
    }

    // Insert our decimal point
    *first = *(first + 1);
    *(first + 1) = '.';
    first = r.ptr;

    if (local_precision == 0)
    {
        --first;
    }

    *first++ = 'p';
    if (exp < 0)
    {
        *first++ = '-';
    }
    else
    {
        *first++ = '+';
    }

    const int abs_exp { (exp < 0) ? -exp : exp };

    if (abs_exp < 10)
    {
        *first++ = '0';
    }

    return to_chars_integer_impl<std::uint32_t>(first, last, static_cast<std::uint32_t>(abs_exp));
}

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4702) // Unreachable code
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars_impl(char* first, char* last, const TargetDecimalType& value, const chars_format fmt = chars_format::general, const int local_precision = -1) noexcept -> to_chars_result
{
    // Sanity check our bounds
    if (first >= last)
    {
        return {last, std::errc::invalid_argument};
    }

    auto abs_value = abs(value);
    constexpr auto max_fractional_value = decimal_val_v<TargetDecimalType> < 64 ?  TargetDecimalType{1, 7} :
                                                          decimal_val_v<TargetDecimalType> < 128 ? TargetDecimalType{1, 16} :
                                                                                                         TargetDecimalType{1, 34};

    constexpr auto min_fractional_value = TargetDecimalType{1, -4};

    // Unspecified precision so we always go with the shortest representation
    if (local_precision == -1)
    {
        switch (fmt)
        {
            case chars_format::general:
                if (abs_value >= 1 && abs_value < max_fractional_value)
                {
                    return to_chars_fixed_impl(first, last, value, fmt);
                }
                else
                {
                    return to_chars_scientific_impl(first, last, value, fmt);
                }
            case chars_format::fixed:
                return to_chars_fixed_impl(first, last, value, fmt);
            case chars_format::scientific:
                return to_chars_scientific_impl(first, last, value, fmt);
            case chars_format::hex:
                return to_chars_hex_impl(first, last, value, local_precision);
            // LCOV_EXCL_START
            default:
                BOOST_DECIMAL_UNREACHABLE;
            // LCOV_EXCL_STOP
        }
    }
    else
    {
        // In this range with general formatting, fixed formatting is the shortest
        if (fmt == chars_format::general && abs_value >= min_fractional_value && abs_value < max_fractional_value)
        {
            return to_chars_fixed_impl(first, last, value, fmt, local_precision);
        }

        if (fmt == chars_format::fixed)
        {
            return to_chars_fixed_impl(first, last, value, fmt, local_precision);
        }
        else if (fmt == chars_format::hex)
        {
            return to_chars_hex_impl(first, last, value, local_precision);
        }
        else
        {
            return to_chars_scientific_impl(first, last, value, fmt, local_precision);
        }
    }

    return to_chars_scientific_impl(first, last, value, fmt, local_precision); // LCOV_EXCL_LINE
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} //namespace detail

BOOST_DECIMAL_EXPORT template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars(char* first, char* last, const TargetDecimalType& value) noexcept -> to_chars_result
{
    return detail::to_chars_impl(first, last, value);
}

BOOST_DECIMAL_EXPORT template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars(char* first, char* last, const TargetDecimalType& value, chars_format fmt) noexcept -> to_chars_result
{
    return detail::to_chars_impl(first, last, value, fmt);
}

BOOST_DECIMAL_EXPORT template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars(char* first, char* last, const TargetDecimalType& value, chars_format fmt, int precision) noexcept -> to_chars_result
{
    if (precision < 0)
    {
        precision = 6;
    }

    return detail::to_chars_impl(first, last, value, fmt, precision);
}

#ifdef BOOST_DECIMAL_HAS_STD_CHARCONV

BOOST_DECIMAL_EXPORT template <typename DecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars(char* first, char* last, DecimalType value, std::chars_format fmt)
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::to_chars_result)
{
    to_chars_result boost_r {};
    switch (fmt)
    {
        case std::chars_format::scientific:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::scientific);
            break;
        case std::chars_format::fixed:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::fixed);
            break;
        case std::chars_format::hex:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::hex);
            break;
        case std::chars_format::general:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::general);
            break;
        // LCOV_EXCL_START
        default:
            BOOST_DECIMAL_UNREACHABLE;
        // LCOV_EXCL_STOP
    }

    return std::to_chars_result {boost_r.ptr, boost_r.ec};
}

BOOST_DECIMAL_EXPORT template <typename DecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars(char* first, char* last, DecimalType value, std::chars_format fmt, int precision)
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::to_chars_result)
{
    if (precision < 0)
    {
        precision = 6;
    }

    to_chars_result boost_r {};
    switch (fmt)
    {
        case std::chars_format::scientific:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::scientific, precision);
            break;
        case std::chars_format::fixed:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::fixed, precision);
            break;
        case std::chars_format::hex:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::hex, precision);
            break;
        case std::chars_format::general:
            boost_r = detail::to_chars_impl(first, last, value, chars_format::general, precision);
            break;
        // LCOV_EXCL_START
        default:
            BOOST_DECIMAL_UNREACHABLE;
        // LCOV_EXCL_STOP
    }

    return std::to_chars_result {boost_r.ptr, boost_r.ec};
}

#endif // BOOST_DECIMAL_HAS_STD_CHARCONV

BOOST_DECIMAL_EXPORT template <typename T>
struct limits
{
    static constexpr int max_chars = boost::decimal::detail::max_string_length_v<T>;
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <typename T> constexpr int limits<T>::max_chars;

#endif

} //namespace decimal
} //namespace boost

#endif

#endif //BOOST_DECIMAL_CHARCONV_HPP
