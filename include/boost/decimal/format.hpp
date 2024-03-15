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
    int precision = 6;
    boost::decimal::chars_format fmt = boost::decimal::chars_format::general;

    if (it == ctx.end())
    {
        return std::make_tuple(precision, fmt, it);
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

        if (*it != 'f')
        {
            throw std::format_error("Invalid format");
        }
        ++it;
    }

    if (*it != '}')
    {
        throw std::format_error("Invalid format");
    }

    return std::make_tuple(precision, fmt, it);
};
};

template <>
struct std::formatter<boost::decimal::decimal32>
{
    int precision;
    boost::decimal::chars_format fmt;

    constexpr auto parse(const std::basic_format_parse_context<char>& context)
    {
        auto res {boost::decimal::detail::parse_impl(context)};
        precision = std::get<0>(res);
        fmt = std::get<1>(res);
        return std::get<2>(res);
    }

    template <typename OutputIterator>
    auto format(const boost::decimal::decimal32& v, std::basic_format_context<OutputIterator, char>& context) const
    {
        auto&& out = context.out();
        char buffer[128U];
        const auto r = to_chars(buffer, buffer + sizeof(buffer), v, fmt, precision);
        *r.ptr = '\0';
        out = std::copy(buffer, r.ptr, out);
        return out;
    }
};

#endif

#endif //BOOST_DECIMAL_FORMAT_HPP
