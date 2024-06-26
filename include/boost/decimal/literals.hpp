// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_LITERALS_HPP
#define BOOST_DECIMAL_LITERALS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/charconv.hpp>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstring>
#endif

namespace boost { namespace decimal {

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DF(const char* str) -> decimal32
{
    decimal32 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _df(const char* str) -> decimal32
{
    decimal32 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DF(const char* str, std::size_t) -> decimal32
{
    decimal32 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _df(const char* str, std::size_t) -> decimal32
{
    decimal32 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DF(unsigned long long v) -> decimal32
{
    return decimal32{v};
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _df(unsigned long long v) -> decimal32
{
    return decimal32{v};
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DD(const char* str) -> decimal64
{
    decimal64 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _dd(const char* str) -> decimal64
{
    decimal64 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DD(const char* str, std::size_t) -> decimal64
{
    decimal64 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _dd(const char* str, std::size_t) -> decimal64
{
    decimal64 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DD(unsigned long long v) -> decimal64
{
    return decimal64{v};
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _dd(unsigned long long v) -> decimal64
{
    return decimal64{v};
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DL(const char* str) -> decimal128
{
    decimal128 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _dl(const char* str) -> decimal128
{
    decimal128 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DL(const char* str, std::size_t) -> decimal128
{
    decimal128 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _dl(const char* str, std::size_t) -> decimal128
{
    decimal128 d;
    from_chars(str, str + detail::strlen(str), d);
    return d;
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _DL(unsigned long long v) -> decimal128
{
    return decimal128{v};
}

BOOST_DECIMAL_EXPORT constexpr auto operator  "" _dl(unsigned long long v) -> decimal128
{
    return decimal128{v};
}

} // namespace decimal
} // namespace boost

#endif

#endif // BOOST_DECIMAL_LITERALS_HPP
