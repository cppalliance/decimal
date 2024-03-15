// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FORMAT_HPP
#define BOOST_DECIMAL_FORMAT_HPP

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)) && __has_include(<format>) && !defined(BOOST_DECIMAL_DISABLE_CLIB)

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/charconv.hpp>
#include <algorithm>
#include <format>
#include <iostream>
#include <iomanip>
#include <string>
#include <tuple>
#include <cctype>

// Default :g
// Fixed :f
// Scientific :3
// Hex :a
//
// Capital letter for any of the above leads to all characters being uppercase

namespace boost::decimal::detail {

template <typename ParseContext>
constexpr auto parse_impl(ParseContext& ctx)
{
    auto it {ctx.begin()};
    ++it;
    int precision = 6;
    boost::decimal::chars_format fmt = boost::decimal::chars_format::general;
    bool is_upper = false;
    int padding_digits = 0;

    if (it == ctx.end())
    {
        return std::make_tuple(precision, fmt, is_upper, padding_digits, it);
    }

    while (*it >= '0' && *it <= '9')
    {
        padding_digits = padding_digits * 10 + *it;
        ++it;
    }

    if (*it == ':')
    {
        ++it;
    }

    if (*it == '.')
    {
        ++it;
        precision = 0;
        while (*it >= '0' && *it <= '9')
        {
            precision = precision * 10 + *it;
            ++it;
        }

        switch (*it)
        {
            case 'G':
                is_upper = true;
                [[fallthrough]];
            case 'g':
                fmt = chars_format::general;
                break;

            case 'F':
                [[fallthrough]];
            case 'f':
                fmt = chars_format::fixed;
                break;

            case 'E':
                is_upper = true;
                [[fallthrough]];
            case 'e':
                fmt = chars_format::scientific;
                break;

            case 'A':
                is_upper = true;
                [[fallthrough]];
            case 'a':
                fmt = chars_format::hex;
                break;

            default:
                throw std::format_error("Invalid format");
        }
        ++it;
    }

    if (*it != '}')
    {
        throw std::format_error("Invalid format");
    }

    return std::make_tuple(precision, fmt, is_upper, padding_digits, it);
};

} //namespace boost::decimal::detail

template <>
struct std::formatter<boost::decimal::decimal32>
{
    int precision;
    boost::decimal::chars_format fmt;
    bool is_upper;
    int padding_digits;

    constexpr auto parse(const std::basic_format_parse_context<char>& context)
    {
        const auto res {boost::decimal::detail::parse_impl(context)};

        precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);

        return std::get<4>(res);
    }

    template <typename OutputIterator>
    auto format(const boost::decimal::decimal32& v, std::basic_format_context<OutputIterator, char>& context) const
    {
        auto&& out = context.out();
        char buffer[128U];
        const auto r = to_chars(buffer, buffer + sizeof(buffer), v, fmt, precision);
        *r.ptr = '\0';
        std::string s(buffer);

        if (is_upper)
        {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), '0');
        }

        out = std::copy(s.begin(), s.end(), out);
        return out;
    }
};

#endif

#endif //BOOST_DECIMAL_FORMAT_HPP
