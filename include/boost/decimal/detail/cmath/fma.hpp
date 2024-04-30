// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/detail/config.hpp>

namespace boost {
namespace decimal {

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return fmad32(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64
{
    return fmad64(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal128 x, decimal128 y, decimal128 z) noexcept -> decimal128
{
    return fmad128(x, y, z);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
