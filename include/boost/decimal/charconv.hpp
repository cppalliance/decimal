// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CHARCONV_HPP
#define BOOST_DECIMAL_CHARCONV_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/from_chars_result.hpp>
#include <boost/decimal/detail/chars_format.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/to_chars_result.hpp>
#include <boost/decimal/detail/to_chars_integer_impl.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>
#include <cstdint>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

namespace boost {
namespace decimal {

// ---------------------------------------------------------------------------------------------------------------------
// from_chars and implementation
// ---------------------------------------------------------------------------------------------------------------------

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto from_chars_general_impl(const char* first, const char* last, TargetDecimalType& value) noexcept -> from_chars_result
{
    using significand_type = std::conditional_t<std::is_same<TargetDecimalType, decimal128>::value, detail::uint128, std::uint64_t>;

    if (first >= last)
    {
        return {first, std::errc::invalid_argument};
    }

    bool sign {};
    significand_type significand {};
    std::int32_t expval {};

    auto r {detail::parser(first, last, sign, significand, expval)};

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

constexpr auto from_chars(const char* first, const char* last, decimal32& value, chars_format fmt = chars_format::general) noexcept
{
    BOOST_DECIMAL_ASSERT_MSG(fmt != chars_format::hex, "Hex is not yet implemented");
    static_cast<void>(fmt);
    return detail::from_chars_general_impl(first, last, value);
}

constexpr auto from_chars(const char* first, const char* last, decimal64& value, chars_format fmt = chars_format::general) noexcept
{
    BOOST_DECIMAL_ASSERT_MSG(fmt != chars_format::hex, "Hex is not yet implemented");
    static_cast<void>(fmt);
    return detail::from_chars_general_impl(first, last, value);
}

constexpr auto from_chars(const char* first, const char* last, decimal128& value, chars_format fmt = chars_format::general) noexcept
{
    BOOST_DECIMAL_ASSERT_MSG(fmt != chars_format::hex, "Hex is not yet implemented");
    static_cast<void>(fmt);
    return detail::from_chars_general_impl(first, last, value);
}

// ---------------------------------------------------------------------------------------------------------------------
// to_chars and implementation
// ---------------------------------------------------------------------------------------------------------------------

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
BOOST_DECIMAL_CONSTEXPR auto to_chars_scientific_impl(char* first, char* last, const TargetDecimalType& value, int precision = -1) -> to_chars_result
{
    // TODO(mborland): Add precision support
    static_cast<void>(precision);

    // Sanity check our bounds
    if (first >= last)
    {
        return {last, std::errc::value_too_large};
    }

    auto buffer_len = last - first;

    if (signbit(value))
    {
        --buffer_len;
        *first++ = '-';
    }

    switch (fpclassify(value))
    {
        case FP_INFINITE:
            if (buffer_len >= 3)
            {
                boost::decimal::detail::memcpy(first, "inf", 3U);
                return {first + 3U, std::errc()};
            }
            else
            {
                return {last, std::errc::value_too_large};
            }
        case FP_ZERO:
            if (buffer_len >= 7)
            {
                boost::decimal::detail::memcpy(first, "0.0e+00", 7U);
                return {first + 7U, std::errc()};
            }
        case FP_NAN:
            if (issignaling(value) && buffer_len >= 9)
            {
                boost::decimal::detail::memcpy(first, "nan(snan)", 9U);
                return {first + 9U, std::errc()};
            }
            else if (buffer_len >= 3)
            {
                boost::decimal::detail::memcpy(first, "nan", 3U);
                return {first + 3U, std::errc()};
            }
            else
            {
                return {last, std::errc::value_too_large};
            }
        default:
            static_cast<void>(precision);
    }

    int exp {};
    auto significand {frexp10(value, &exp)};

    using uint_type = std::conditional_t<std::is_same<TargetDecimalType, decimal128>::value, uint128, std::uint64_t>;

    // Offset the value of first by 1 so that we can copy the leading digit and insert a decimal point
    auto r = to_chars_integer_impl<uint_type, uint_type>(first + 1, last, significand, 10);

    // Only real reason we will hit this is a buffer overflow
    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    // Insert our decimal point
    *first = *(first + 1);
    *(first + 1) = '.';
    first = r.ptr;

    // Insert the exponent character
    *first++ = 'e';
    const int abs_exp {std::abs(exp)};
    if (exp < 0)
    {
        *first++ = '-';
    }
    else
    {
        *first++ = '+';
    }

    // Always give 2 digits in the exp (ex. 2.0e+09)
    if (abs_exp < 9)
    {
        *first++ = '0';
    }

    r = to_chars_integer_impl<int, unsigned>(first, last, abs_exp, 10);
    if (BOOST_DECIMAL_UNLIKELY(!r))
    {
        return r; // LCOV_EXCL_LINE
    }

    return {r.ptr, std::errc()};
}

} //namespace detail

} //namespace decimal
} //namespace boost

#endif

#endif //BOOST_DECIMAL_CHARCONV_HPP
