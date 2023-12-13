// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FORMAT_HPP
#define BOOST_DECIMAL_FORMAT_HPP

#if __cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) && __has_include(<format>) && !defined(BOOST_DECIMAL_DISABLE_CLIB)

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <format>
#include <iostream>
#include <iomanip>
#include <string>
#include <tuple>

// See for general impl
// https://en.cppreference.com/w/cpp/utility/format/formatter

namespace boost::decimal::detail {

template <typename ParseContext>
constexpr auto parse_impl(ParseContext& ctx)
{
    auto it {ctx.begin()};
    int precision = 5;

    if (it == ctx.end())
    {
        return std::make_tuple(precision, it);
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

    return std::make_tuple(precision, it);
};
};

template <>
struct std::formatter<boost::decimal::decimal32>
{
    int precision {};

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        auto res {boost::decimal::detail::parse_impl(ctx)};
        precision = std::get<0>(res);
        return std::get<1>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal32& v, FormatContext& ctx)
    {
        std::ostringstream out;
        out << std::setprecision(precision) << v;

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

template <>
struct std::formatter<boost::decimal::decimal64>
{
    int precision;

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        auto res {boost::decimal::detail::parse_impl(ctx)};
        precision = std::get<0>(res);
        return std::get<1>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal32& v, FormatContext& ctx)
    {
        std::ostringstream out;
        out << std::setprecision(precision) << v;

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

template <>
struct std::formatter<boost::decimal::decimal128>
{
    int precision;

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        auto res {boost::decimal::detail::parse_impl(ctx)};
        precision = std::get<0>(res);
        return std::get<1>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal32& v, FormatContext& ctx)
    {
        std::ostringstream out;
        out << std::setprecision(precision) << v;

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};


#endif

#endif //BOOST_DECIMAL_FORMAT_HPP
