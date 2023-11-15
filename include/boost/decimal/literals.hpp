// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_LITERALS_HPP
#define BOOST_DECIMAL_LITERALS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#include <type_traits>
#include <cstring>

namespace boost { namespace decimal {

constexpr auto operator  "" _DF(const char* str) -> decimal32
{
    char* endptr {};
    return strtod32(str, &endptr);
}

constexpr auto operator  "" _df(const char* str) -> decimal32
{
    char* endptr {};
    return strtod32(str, &endptr);
}

constexpr auto operator  "" _DF(unsigned long long v) -> decimal32
{
    return decimal32{v};
}

constexpr auto operator  "" _df(unsigned long long v) -> decimal32
{
    return decimal32{v};
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto operator  "" _DF(long double v) -> decimal32
{
    return decimal32{v};
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto operator  "" _df(long double v) -> decimal32
{
    return decimal32{v};
}

constexpr auto operator  "" _DD(const char* str) -> decimal64
{
    char* endptr {};
    return strtod64(str, &endptr);
}

constexpr auto operator  "" _dd(const char* str) -> decimal64
{
    char* endptr {};
    return strtod64(str, &endptr);
}

constexpr auto operator  "" _DD(unsigned long long v) -> decimal64
{
    return decimal64{v};
}

constexpr auto operator  "" _dd(unsigned long long v) -> decimal64
{
    return decimal64{v};
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto operator  "" _DD(long double v) -> decimal64
{
    return decimal64{v};
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto operator  "" _dd(long double v) -> decimal64
{
    return decimal64{v};
}

} // namespace decimal
} // namespace boost

#endif

#endif // BOOST_DECIMAL_LITERALS_HPP
