// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DECIMAL_DETAIL_TO_STRING
#define BOOST_DECIMAL_DETAIL_TO_STRING

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/concepts.hpp>

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

// GCC UBSAN warns of format truncation from the constexpr calculation of the format
// This warning was added in GCC 7.1
#if __GNUC__ >= 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

namespace detail {

template <typename DecimalType, typename Integer, std::enable_if_t<!std::is_same<DecimalType, decimal128>::value, bool> = true>
void print_buffer(char* buffer, std::size_t buffer_size, const char* format, Integer significand)
{
    std::snprintf(buffer, buffer_size, format, significand);
}

template <typename DecimalType, typename Integer, std::enable_if_t<std::is_same<DecimalType, decimal128>::value, bool> = true>
void print_buffer(char* buffer, std::size_t buffer_size, const char*, Integer significand)
{
    char local_buffer [64];
    const auto p {detail::emulated128_to_buffer(local_buffer, significand)};
    const auto print_size {static_cast<std::size_t>(local_buffer + 64 - p)};
    if (print_size <= buffer_size)
    {
        std::memcpy(buffer, p, print_size);
    }
}

} //namespace detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
auto to_string(DecimalType value) -> std::string
{
    std::string res;

    const bool d_isneg {signbit(value)};
    
    if (issignaling(value))
    {
        if (d_isneg)
        {
            res += "-";
        }

        res += "nan(snan)";
        return res;
    }

    if (isnan(value)) // only quiet NaNs left
    {
        if (d_isneg)
        {
            res += "-nan(ind)";
        }
        else
        {
            res += "nan";
        }

        return res;
    }

    if (isinf(value))
    {
        if (d_isneg)
        {
            res += "-";
        }

        res += "inf";
        return res;
    }

    auto precision {std::numeric_limits<DecimalType>::digits10};

    char buffer[detail::precision_v<DecimalType> + 6] {}; // Sign + Precision + decimal point + e + sign + null terminator

    if (value.isneg() == 1)
    {
        res += "-";
    }

    constexpr auto format {std::is_same<DecimalType, decimal32>::value ? "%" PRIu32 : "%" PRIu64};
    auto exp {value.biased_exponent()};
    auto significand {value.full_significand()};

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
    res += buffer;

    // Offset will adjust the exponent to compensate for adding the decimal point
    const auto offset {detail::num_digits(significand) - 1};
    if (offset == 0)
    {
        res += "0";
    }

    res += "e";
    auto print_exp {exp + offset};

    if (print_exp < 0)
    {
        res += "-";
        print_exp = -print_exp;
    }
    else
    {
        res += "+";
    }

    if (print_exp < 10)
    {
        res += "0";
    }

    res += std::to_string(print_exp);

    return res;
}

#if __GNUC__ >= 7
#  pragma GCC diagnostic pop
#endif

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_TO_STRING
