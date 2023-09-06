// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_LITERALS_HPP
#define BOOST_DECIMAL_LITERALS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <string>

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

}}

#endif // BOOST_DECIMAL_LITERALS_HPP
