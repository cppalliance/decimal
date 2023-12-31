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
    using significand_type = std::conditional_t<std::is_same<DecimalType, decimal128>::value, detail::uint128, std::uint64_t>;

    char buffer[1024] {}; // What should be an unreasonably high maximum
    is >> buffer;

    bool sign {};
    significand_type significand {};
    std::int32_t expval {};
    const auto buffer_len {std::strlen(buffer)};

    if (buffer_len == 0)
    {
        errno = EINVAL;
        return is;
    }

    const auto r {detail::parser(buffer, buffer + buffer_len, sign, significand, expval)};

    if (r.ec != std::errc{})
    {
        if (r.ec == std::errc::not_supported)
        {
            if (significand)
            {
                d = std::numeric_limits<DecimalType>::signaling_NaN();
            }
            else
            {
                d = std::numeric_limits<DecimalType>::quiet_NaN();
            }
        }
        else if (r.ec == std::errc::value_too_large)
        {
            d = std::numeric_limits<DecimalType>::infinity();
        }
        else
        {
            d = std::numeric_limits<DecimalType>::signaling_NaN();
            errno = static_cast<int>(r.ec);
        }
    }
    else
    {
        d = DecimalType{significand, expval, sign};
    }

    return is;
}

// GCC UBSAN warns of format truncation from the constexpr calculation of the format
// This warning was added in GCC 7.1
#if __GNUC__ >= 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

// 3.2.11 Formatted output
template <typename charT, typename traits, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
auto operator<<(std::basic_ostream<charT, traits>& os, const DecimalType& d)
    -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_ostream<charT, traits>&>
{
    const bool d_isneg {signbit(d)};
    
    if (issignaling(d))
    {
        if (d_isneg)
        {
            os << "-";
        }

        os << "nan(snan)";
        return os;
    }
    else if (isnan(d)) // only quiet NaNs left
    {
        if (d_isneg)
        {
            os << "-nan(ind)";
        }
        else
        {
            os << "nan";
        }

        return os;
    }
    else if (isinf(d))
    {
        if (d_isneg)
        {
            os << "-";
        }

        os << "inf";
        return os;
    }

    // const auto flags {os.flags()};
    auto precision {os.precision()};
    if (precision > std::numeric_limits<DecimalType>::digits10)
    {
        precision = std::numeric_limits<DecimalType>::digits10;
    }

    char buffer[detail::precision_v<DecimalType> + 6] {}; // Sign + Precision + decimal point + e + sign + null terminator

    if (d.isneg() == 1)
    {
        os << "-";
    }

    constexpr auto format {std::is_same<DecimalType, decimal32>::value ? "%" PRIu32 : "%" PRIu64};
    auto exp {d.biased_exponent()};
    auto significand {d.full_significand()};

    auto significand_digits {detail::num_digits(significand)};
    const bool reduced {significand_digits > precision};
    while (significand_digits > precision + 1)
    {
        significand /= 10;
        ++exp;
        --significand_digits;
    }

    if (reduced)
    {
        exp += detail::fenv_round<DecimalType>(significand, d < 0);
    }

    // Print the significand into the buffer so that we can insert the decimal point
    detail::print_buffer<DecimalType>(buffer, sizeof(buffer), format, significand);
    std::memmove(buffer + 2, buffer + 1, static_cast<std::size_t>(precision - 1));
    std::memset(buffer + 1, '.', 1);
    os << buffer;

    // Offset will adjust the exponent to compensate for adding the decimal point
    const auto offset {detail::num_digits(significand) - 1};
    if (offset == 0)
    {
        os << "0";
    }

    os << "e";
    auto print_exp {exp + offset};

    if (print_exp < 0)
    {
        os << "-";
        print_exp = -print_exp;
    }
    else
    {
        os << "+";
    }

    if (print_exp < 10)
    {
        os << "0";
    }

    os << print_exp;

    return os;
}

#if __GNUC__ >= 7
#  pragma GCC diagnostic pop
#endif

} //namespace decimal
} //namespace boost

#endif // BOOST_DECIMAL_DISABLE_CLIB

#endif //BOOST_DECIMAL_DETAIL_IO_HPP
