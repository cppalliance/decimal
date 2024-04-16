// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDIO_HPP
#define BOOST_DECIMAL_CSTDIO_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/locale_conversion.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/charconv.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cctype>
#endif

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

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
            params.precision = params.precision * 10 + digit_from_char(*iter);
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
            return params;
        }
        else if (*iter == 'D')
        {
            params.return_type = decimal_type::decimal128;
            ++iter;
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
            params.fmt = chars_format::scientific;
            break;
        case 'f':
            params.fmt = chars_format::fixed;
            break;
        case 'A':
            params.upper_case = true;
        case 'a':
            params.fmt = chars_format::hex;
            break;
        default:
            // Invalid specifier
            return params;
    }

    return params;
}

inline void make_uppercase(char* first, const char* last) noexcept
{
    while (first != last)
    {
        if ((*first >= 'a' && *first <= 'f') || *first == 'p')
        {
            *first = static_cast<char>(std::toupper(static_cast<int>(*first)));
        }
        ++first;
    }
}

} // namespace detail

template <typename T>
inline auto snprintf(char* buffer, std::size_t buf_size, const char* format, T value) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int)
{
    if (buffer == nullptr || format == nullptr)
    {
        return -1;
    }

    auto params = detail::parse_format(format);

    to_chars_result r;
    switch (params.return_type)
    {
        case detail::decimal_type::decimal32:
            r = to_chars(buffer, buffer + buf_size, static_cast<decimal32>(value), params.fmt, params.precision);
            break;
        case detail::decimal_type::decimal64:
            r = to_chars(buffer, buffer + buf_size, static_cast<decimal64>(value), params.fmt, params.precision);
            break;
        default:
            r = to_chars(buffer, buffer + buf_size, static_cast<decimal128>(value), params.fmt, params.precision);
            break;
    }

    if (!r)
    {
        errno = static_cast<int>(r.ec);
        return -1;
    }

    *r.ptr = '\0';
    if (params.upper_case)
    {
        detail::make_uppercase(buffer, r.ptr);
    }

    detail::convert_string_to_local_locale(buffer);

    return static_cast<int>(r.ptr - buffer);
}

template <typename T>
inline auto sprintf(char* buffer, const char* format, T value) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int)
{
    return snprintf(buffer, sizeof(buffer), format, value);
}

} // namespace decimal
} // namespace boost

#endif // #if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#endif //BOOST_DECIMAL_CSTDIO_HPP
