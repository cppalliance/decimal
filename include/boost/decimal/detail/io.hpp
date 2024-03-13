// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_IO_HPP
#define BOOST_DECIMAL_DETAIL_IO_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/to_string.hpp>
#include <boost/decimal/charconv.hpp>
#include <system_error>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#include <cerrno>
#include <cstring>
#include <cinttypes>
#include <limits>
#include <ios>
#include <iostream>
#include <system_error>
#include <type_traits>

namespace boost {
namespace decimal {

// 3.2.10 Formatted input:
template <typename charT, typename traits, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
auto operator>>(std::basic_istream<charT, traits>& is, DecimalType& d)
    -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_istream<charT, traits>&>
{
    charT t_buffer[1024] {}; // What should be an unreasonably high maximum
    is >> t_buffer;

    char buffer[1024] {};

    BOOST_DECIMAL_IF_CONSTEXPR (!std::is_same<charT, char>::value)
    {
        auto first = buffer;
        auto t_first = t_buffer;
        auto t_buffer_end = t_buffer + std::strlen(t_buffer);

        while (t_first != t_buffer_end)
        {
            *first++ = static_cast<char>(*t_first++);
        }
    }
    else
    {
        std::memcpy(buffer, t_buffer, sizeof(t_buffer));
    }

    chars_format fmt = chars_format::general;
    const auto flags {is.flags()};
    if (flags & std::ios_base::scientific)
    {
        fmt = chars_format::scientific;
    }
    else if (flags & std::ios_base::hex)
    {
        fmt = chars_format::hex;
    }
    else if (flags & std::ios_base::fixed)
    {
        fmt = chars_format::fixed;
    }

    auto r = from_chars(buffer, buffer + std::strlen(buffer), d, fmt);

    if (BOOST_DECIMAL_UNLIKELY(r.ec == std::errc::not_supported))
    {
        d = std::numeric_limits<DecimalType>::signaling_NaN(); // LCOV_EXCL_LINE
    }

    return is;
}

// GCC UBSAN warns of format truncation from the constexpr calculation of the format
// This warning was added in GCC 7.1
#if defined(__GNUC__) && __GNUC__ >= 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

// 3.2.11 Formatted output
template <typename charT, typename traits, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
auto operator<<(std::basic_ostream<charT, traits>& os, const DecimalType& d)
    -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_ostream<charT, traits>&>
{
    chars_format fmt = chars_format::general;
    const auto flags {os.flags()};
    if (flags & std::ios_base::scientific)
    {
        fmt = chars_format::scientific;
    }
    else if (flags & std::ios_base::hex)
    {
        fmt = chars_format::hex;
    }
    else if (flags & std::ios_base::fixed)
    {
        fmt = chars_format::fixed;
    }

    auto precision {os.precision()};
    if (precision > std::numeric_limits<DecimalType>::digits10)
    {
        precision = std::numeric_limits<DecimalType>::digits10;
    }

    char buffer[precision + 8]; // Sign + Precision + decimal point + e + sign + min 2 characters + null terminator
    auto r = to_chars(buffer, buffer + sizeof(buffer), d, fmt, precision);
    *r.ptr++ = '\0';

    BOOST_DECIMAL_IF_CONSTEXPR (!std::is_same<charT, char>::value)
    {
        charT t_buffer[precision + 8];

        auto first = buffer;
        auto t_first = t_buffer;
        while (first != r.ptr)
        {
            *t_first++ = static_cast<charT>(*first++);
        }

        os << t_buffer;
    }
    else
    {
        os << buffer;
    }

    return os;
}

#if defined(__GNUC__) && __GNUC__ >= 7
#  pragma GCC diagnostic pop
#endif

} //namespace decimal
} //namespace boost

#endif // BOOST_DECIMAL_DISABLE_CLIB

#endif //BOOST_DECIMAL_DETAIL_IO_HPP
