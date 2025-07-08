// Copyright 2023 - 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FMT_FORMAT_HPP
#define BOOST_DECIMAL_FMT_FORMAT_HPP

#if __has_include(<fmt/format.h>)

#include <fmt/format.h>
#include <fmt/base.h>
#include <boost/decimal/charconv.hpp>
#include <algorithm>
#include <format>
#include <iostream>
#include <string>
#include <tuple>
#include <cctype>

namespace boost {
namespace decimal {
namespace detail {
namespace fmt_detail {

template <typename ParseContext>
constexpr auto parse_impl(ParseContext &ctx)
{
    auto it {ctx.begin()};
    int ctx_precision = 6;
    boost::decimal::chars_format fmt = boost::decimal::chars_format::general;
    bool is_upper = false;
    int padding_digits = 0;

    // Check for a padding character
    while (it != ctx.end() && *it >= '0' && *it <= '9')
    {
        padding_digits = padding_digits * 10 + (*it - '0');
        ++it;
    }

    // If there is a . then we need to capture the precision argument
    if (*it == '.')
    {
        ++it;
        ctx_precision = 0;
        while (it != ctx.end() && *it >= '0' && *it <= '9')
        {
            ctx_precision = ctx_precision * 10 + (*it - '0');
            ++it;
        }
    }

    // Lastly we capture the format to include if it's upper case
    if (it != ctx.end() && *it != '}')
    {
        switch (*it)
        {
            case 'G':
                is_upper = true;
                fmt = chars_format::general;
                break;
            case 'g':
                fmt = chars_format::general;
                break;

            case 'F':
                is_upper = true;
                fmt = chars_format::fixed;
                break;
            case 'f':
                fmt = chars_format::fixed;
                break;

            case 'E':
                is_upper = true;
                fmt = chars_format::scientific;
                break;
            case 'e':
                fmt = chars_format::scientific;
                break;

            case 'A':
                is_upper = true;
                fmt = chars_format::hex;
                break;
            case 'a':
                fmt = chars_format::hex;
                break;
                // LCOV_EXCL_START
            default:
                throw std::logic_error("Invalid format specifier");
                // LCOV_EXCL_STOP
        }
    }

    ++it;

    return std::make_tuple(ctx_precision, fmt, is_upper, padding_digits, it);
}

}
}
}
} // Namespace boost::decimal::detail::fmt_detail

// All the following are identical except the type since fmt does not accept SFINAE
// If an edit is made to one copy and past it down the line

template <>
struct fmt::formatter<boost::decimal::decimal32>
{
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
        const auto res {boost::decimal::detail::fmt_detail::parse_impl(ctx)};

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
        const auto r = boost::decimal::to_chars(buffer.data(), buffer.data() + buffer.size(), v, fmt, ctx_precision);

        std::string s(buffer.data(), static_cast<std::size_t>(r.ptr - buffer.data()));

        if (is_upper)
        {
            #ifdef _MSC_VER
            #  pragma warning(push)
            #  pragma warning(disable : 4244)
            #endif

            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });

            #ifdef _MSC_VER
            #  pragma warning(pop)
            #endif
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), ' ');
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct fmt::formatter<boost::decimal::decimal32_fast>
{
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
        const auto res {boost::decimal::detail::fmt_detail::parse_impl(ctx)};

        ctx_precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);

        return std::get<4>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal32_fast &v, FormatContext &ctx) const
    {
        auto out = ctx.out();
        std::array<char, 128> buffer {};
        const auto r = boost::decimal::to_chars(buffer.data(), buffer.data() + buffer.size(), v, fmt, ctx_precision);

        std::string s(buffer.data(), static_cast<std::size_t>(r.ptr - buffer.data()));

        if (is_upper)
        {
            #ifdef _MSC_VER
            #  pragma warning(push)
            #  pragma warning(disable : 4244)
            #endif

            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });

            #ifdef _MSC_VER
            #  pragma warning(pop)
            #endif
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), ' ');
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct fmt::formatter<boost::decimal::decimal64>
{
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
        const auto res {boost::decimal::detail::fmt_detail::parse_impl(ctx)};

        ctx_precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);

        return std::get<4>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal64 &v, FormatContext &ctx) const
    {
        auto out = ctx.out();
        std::array<char, 128> buffer {};
        const auto r = boost::decimal::to_chars(buffer.data(), buffer.data() + buffer.size(), v, fmt, ctx_precision);

        std::string s(buffer.data(), static_cast<std::size_t>(r.ptr - buffer.data()));

        if (is_upper)
        {
            #ifdef _MSC_VER
            #  pragma warning(push)
            #  pragma warning(disable : 4244)
            #endif

            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });

            #ifdef _MSC_VER
            #  pragma warning(pop)
            #endif
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), ' ');
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct fmt::formatter<boost::decimal::decimal64_fast>
{
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
        const auto res {boost::decimal::detail::fmt_detail::parse_impl(ctx)};

        ctx_precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);

        return std::get<4>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal64_fast &v, FormatContext &ctx) const
    {
        auto out = ctx.out();
        std::array<char, 128> buffer {};
        const auto r = boost::decimal::to_chars(buffer.data(), buffer.data() + buffer.size(), v, fmt, ctx_precision);

        std::string s(buffer.data(), static_cast<std::size_t>(r.ptr - buffer.data()));

        if (is_upper)
        {
            #ifdef _MSC_VER
            #  pragma warning(push)
            #  pragma warning(disable : 4244)
            #endif

            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });

            #ifdef _MSC_VER
            #  pragma warning(pop)
            #endif
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), ' ');
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct fmt::formatter<boost::decimal::decimal128>
{
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
        const auto res {boost::decimal::detail::fmt_detail::parse_impl(ctx)};

        ctx_precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);

        return std::get<4>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal128 &v, FormatContext &ctx) const
    {
        auto out = ctx.out();
        std::array<char, 128> buffer {};
        const auto r = boost::decimal::to_chars(buffer.data(), buffer.data() + buffer.size(), v, fmt, ctx_precision);

        std::string s(buffer.data(), static_cast<std::size_t>(r.ptr - buffer.data()));

        if (is_upper)
        {
            #ifdef _MSC_VER
            #  pragma warning(push)
            #  pragma warning(disable : 4244)
            #endif

            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });

            #ifdef _MSC_VER
            #  pragma warning(pop)
            #endif
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), ' ');
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct fmt::formatter<boost::decimal::decimal128_fast>
{
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
        const auto res {boost::decimal::detail::fmt_detail::parse_impl(ctx)};

        ctx_precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);

        return std::get<4>(res);
    }

    template <typename FormatContext>
    auto format(const boost::decimal::decimal128_fast &v, FormatContext &ctx) const
    {
        auto out = ctx.out();
        std::array<char, 128> buffer {};
        const auto r = boost::decimal::to_chars(buffer.data(), buffer.data() + buffer.size(), v, fmt, ctx_precision);

        std::string s(buffer.data(), static_cast<std::size_t>(r.ptr - buffer.data()));

        if (is_upper)
        {
            #ifdef _MSC_VER
            #  pragma warning(push)
            #  pragma warning(disable : 4244)
            #endif

            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });

            #ifdef _MSC_VER
            #  pragma warning(pop)
            #endif
        }

        if (s.size() < static_cast<std::size_t>(padding_digits))
        {
            s.insert(s.begin(), static_cast<std::size_t>(padding_digits) - s.size(), ' ');
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

#endif // __has_include(<fmt/format.h>)

#endif // BOOST_DECIMAL_FMT_FORMAT_HPP
