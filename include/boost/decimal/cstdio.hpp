// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDIO_HPP
#define BOOST_DECIMAL_CSTDIO_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/locale_conversion.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/charconv.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

// H is the type modifier used for decimal32
// D is the type modifier used for deciaml64
// DD is the type modifier used for decimal128

namespace boost {
namespace decimal {

namespace detail {

enum class decimal_type : unsigned
{
    decimal32 = 1 << 0,
    decimal64 = 1 << 1,
    decimal128 = 1 << 2
};

struct parameters
{
    int precision;
    chars_format fmt;
    decimal_type return_type;
    bool upper_case;
};

inline auto parse_format(const char* format) -> parameters
{
    // If the format is unspecified or incorrect we will use this as the default values
    parameters params {6, chars_format::general, decimal_type::decimal64, false};

    auto iter = format;
    const auto last = format + std::strlen(format);

    if (iter == last || *iter != '%')
    {
        return params;
    }

    ++iter;
    if (iter == last)
    {
        return params;
    }

    // Case where we have a precision argumet
    if (*iter == '.')
    {
        ++iter;
        if (iter == last)
        {
            return params;
        }

        params.precision = 0;
        while (iter != last && is_integer_char(*iter))
        {
            params.precision = params.precision * 10 + *iter;
            ++iter;
        }

        if (iter == last)
        {
            return params;
        }
    }

    // Now get the type specifier
    if (*iter == 'H')
    {
        params.return_type = decimal_type::decimal32;
        ++iter;
    }
    else if (*iter == 'D')
    {
        ++iter;
        if (iter == last)
        {
            params.return_type = decimal_type::decimal64;
            return params;
        }
        else if (*iter == 'D')
        {
            params.return_type = decimal_type::decimal128;
        }
        else
        {
            params.return_type = decimal_type::decimal64;
        }
    }

    // And lastly the format
    if (iter == last)
    {
        return params;
    }

    switch (*iter)
    {
        case 'G':
            params.upper_case = true;
            break;
        case 'E':
            params.upper_case = true;
        case 'e':
            params.chars_format = chars_format::scientific;
            break;
        case 'f':
            params.chars_format = chars_format::fixed;
            break;
        case 'A':
            params.upper_case = true;
        case 'a':
            params.chars_format = chars_format::hex;
            break;
        default:
            // Invalid specifier
            return params;
    }

    return params;
}

} // namespace detail

} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_CSTDIO_HPP
