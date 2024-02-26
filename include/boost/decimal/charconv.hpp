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
#include <cstdint>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

namespace boost {
namespace decimal {

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

}

constexpr auto from_chars(const char* first, const char* last, decimal32& value, chars_format fmt = chars_format::general) noexcept
{
    BOOST_DECIMAL_ASSERT_MSG(fmt != chars_format::hex, "Hex is not yet implemented");
    return detail::from_chars_general_impl(first, last, value);
}

constexpr auto from_chars(const char* first, const char* last, decimal64& value, chars_format fmt = chars_format::general) noexcept
{
    BOOST_DECIMAL_ASSERT_MSG(fmt != chars_format::hex, "Hex is not yet implemented");
    return detail::from_chars_general_impl(first, last, value);
}

constexpr auto from_chars(const char* first, const char* last, decimal128& value, chars_format fmt = chars_format::general) noexcept
{
    BOOST_DECIMAL_ASSERT_MSG(fmt != chars_format::hex, "Hex is not yet implemented");
    return detail::from_chars_general_impl(first, last, value);
}

} //namespace decimal
} //namespace boost

#endif

#endif //BOOST_DECIMAL_CHARCONV_HPP
