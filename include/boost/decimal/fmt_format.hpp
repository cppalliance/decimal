// Copyright 2023 - 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FMT_FORMAT_HPP
#define BOOST_DECIMAL_FMT_FORMAT_HPP

#if __has_include(<fmt/format.h>) && __has_include(<fmt/base.h>)

#define BOOST_DECIMAL_HAS_FMTLIB_SUPPORT

#include <fmt/format.h>
#include <fmt/base.h>
#include <boost/decimal/detail/config.hpp>
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

enum class sign_option
{
    plus,
    minus,
    space
};

template <typename ParseContext>
constexpr auto parse_impl(ParseContext &ctx)
{
    auto sign_character = sign_option::minus;
    int ctx_precision = 6;
    boost::decimal::chars_format fmt = boost::decimal::chars_format::general;
    bool is_upper = false;
    int padding_digits = 0;
    auto it {ctx.begin()};

    if (it == nullptr)
    {
        return std::make_tuple(ctx_precision, fmt, is_upper, padding_digits, sign_character, it);
    }

    // Check for a sign character
    if (it != ctx.end())
    {
        switch (*it)
        {
            case '-':
                sign_character = sign_option::minus;
                ++it;
                break;
            case '+':
                sign_character = sign_option::plus;
                ++it;
                break;
            case ' ':
                sign_character = sign_option::space;
                ++it;
                break;
            default:
                break;
        }
    }

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
                BOOST_DECIMAL_THROW_EXCEPTION(std::logic_error("Invalid format specifier"));
            // LCOV_EXCL_STOP
        }
        ++it;
    }

    // Verify we're at the closing brace
    if (it != ctx.end() && *it != '}')
    {
        BOOST_DECIMAL_THROW_EXCEPTION(std::logic_error("Expected '}' in format string")); // LCOV_EXCL_LINE
    }

    return std::make_tuple(ctx_precision, fmt, is_upper, padding_digits, sign_character, it);
}

template <typename T>
struct formatter
{
    sign_option sign;
    chars_format fmt;
    int padding_digits;
    int ctx_precision;
    bool is_upper;

    constexpr formatter() : sign{sign_option::minus},
                            fmt{chars_format::general},
                            padding_digits{0},
                            ctx_precision{6},
                            is_upper{false} {}

    constexpr auto parse(fmt::format_parse_context &ctx)
    {
        const auto res {boost::decimal::detail::fmt_detail::parse_impl(ctx)};

        ctx_precision = std::get<0>(res);
        fmt = std::get<1>(res);
        is_upper = std::get<2>(res);
        padding_digits = std::get<3>(res);
        sign = std::get<4>(res);

        return std::get<5>(res);
    }

    template <typename FormatContext>
    auto format(const T& v, FormatContext& ctx) const
    {
        auto out = ctx.out();
        std::array<char, 128> buffer {};
        const auto r = boost::decimal::to_chars(buffer.data(), buffer.data() + buffer.size(), abs(v), fmt, ctx_precision);

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

        switch (sign)
        {
            case sign_option::minus:
                if (v < 0)
                {
                    s.insert(s.begin(), '-');
                }
                break;
            case sign_option::plus:
                if (v > 0)
                {
                    s.insert(s.begin(), '+');
                }
                else
                {
                    s.insert(s.begin(), '-');
                }
                break;
            case sign_option::space:
                if (v > 0)
                {
                    s.insert(s.begin(), ' ');
                }
                else
                {
                    s.insert(s.begin(), '-');
                }
                break;
        }

        return std::copy(s.begin(), s.end(), out);
    }
};

} // namespace fmt_detail
} // namespace detail
} // namespace decimal
} // Namespace boost

template <>
struct fmt::formatter<boost::decimal::decimal32_t>
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
    auto format(const boost::decimal::decimal32_t &v, FormatContext &ctx) const
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
struct fmt::formatter<boost::decimal::decimal_fast32_t>
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
    auto format(const boost::decimal::decimal_fast32_t &v, FormatContext &ctx) const
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
struct fmt::formatter<boost::decimal::decimal64_t>
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
    auto format(const boost::decimal::decimal64_t &v, FormatContext &ctx) const
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
struct fmt::formatter<boost::decimal::decimal_fast64_t>
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
    auto format(const boost::decimal::decimal_fast64_t &v, FormatContext &ctx) const
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
struct fmt::formatter<boost::decimal::decimal128_t>
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
    auto format(const boost::decimal::decimal128_t &v, FormatContext &ctx) const
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
struct fmt::formatter<boost::decimal::decimal_fast128_t>
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
    auto format(const boost::decimal::decimal_fast128_t &v, FormatContext &ctx) const
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
