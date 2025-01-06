// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FORMAT_HPP
#define BOOST_DECIMAL_FORMAT_HPP

// Many compilers seem to have <format> with completly broken support so narrow down our support range
#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)) && !defined(BOOST_DECIMAL_DISABLE_CLIB) && \
    ((defined(__GNUC__) && __GNUC__ >= 13) || (defined(__clang__) && __clang_major__ > 19) || (defined(_MSC_VER) && _MSC_VER >= 1930))

#define BOOST_CRYPT_HAS_FORMAT_SUPPORT

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
constexpr auto parse_impl(ParseContext &ctx)
{
    auto it {ctx.begin()};
    ++it;
    int ctx_precision = 6;
    boost::decimal::chars_format fmt = boost::decimal::chars_format::general;
    bool is_upper = false;
    int padding_digits = 0;

    if (it == ctx.end())
    {
        return std::make_tuple(ctx_precision, fmt, is_upper, padding_digits, it);
    }

    while (it != ctx.end() && *it >= '0' && *it <= '9')
    {
        padding_digits = padding_digits * 10 + (*it - '0');
        ++it;
    }

    if (it != ctx.end() && *it == ':')
    {
        ++it;
    }

    if (it != ctx.end() && *it == '.')
    {
        ++it;
        ctx_precision = 0;
        while (it != ctx.end() && *it >= '0' && *it <= '9')
        {
            ctx_precision = ctx_precision * 10 + (*it - '0');
            ++it;
        }

        if (it == ctx.end())
        {
            throw std::format_error("Unexpected end of format string");
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

    if (it == ctx.end() || *it != '}')
    {
        throw std::format_error("Invalid format");
    }

    return std::make_tuple(ctx_precision, fmt, is_upper, padding_digits, it);
}

} // Namespace boost::decimal::detail

namespace std {

template <>
struct formatter<boost::decimal::decimal32> {
    constexpr formatter() : ctx_precision(6),
                            fmt(boost::decimal::chars_format::general),
                            is_upper(false),
                            padding_digits(0)
    {}

    int ctx_precision;
    boost::decimal::chars_format fmt;
    bool is_upper;
    int padding_digits;

    constexpr auto parse(format_parse_context &ctx)
    {
        const auto res {boost::decimal::detail::parse_impl(ctx)};

        ctx_precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);

        return std::get<4>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal32 &v, FormatContext &ctx) const
    {
        auto out = ctx.out();
        std::array<char, 128> buffer {};
        const auto r = to_chars(buffer.data(), buffer.data() + buffer.size(), v, fmt, ctx_precision);

        std::string_view sv(buffer.data(), static_cast<std::size_t>(r.ptr - buffer.data()));
        std::string s(sv);

        if (is_upper)
        {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), '0');
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

} // Namespace std

#endif

#endif //BOOST_DECIMAL_FORMAT_HPP
