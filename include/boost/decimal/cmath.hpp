// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CMATH_HPP
#define BOOST_DECIMAL_CMATH_HPP

// TODO(mborland): Allow conversion between decimal types via a promotion system
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/cmath/fmax.hpp>
#include <boost/decimal/detail/cmath/fmin.hpp>
#include <boost/decimal/detail/cmath/isgreater.hpp>
#include <boost/decimal/detail/cmath/isless.hpp>
#include <boost/decimal/detail/cmath/isunordered.hpp>
#include <boost/decimal/detail/cmath/trunc.hpp>

namespace boost { namespace decimal {

// Generic overloads

constexpr auto floor(decimal32 num) noexcept -> decimal32
{
    return floord32(num);
}

constexpr auto ceil(decimal32 num) noexcept -> decimal32
{
    return ceild32(num);
}

constexpr auto frexp10(decimal32 num, int *exp) noexcept -> std::int32_t
{
    return frexp10d32(num, exp);
}

constexpr auto scalbn(decimal32 num, int exp) noexcept -> decimal32
{
    return scalbnd32(num, exp);
}

constexpr auto scalbln(decimal32 num, long exp) noexcept -> decimal32
{
    return scalblnd32(num, exp);
}

constexpr auto fmod(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return fmodd32(lhs, rhs);
}

constexpr auto copysign(decimal32 mag, decimal32 sgn) noexcept -> decimal32
{
    return copysignd32(mag, sgn);
}

constexpr auto fma(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return fmad32(x, y, z);
}

}} // Namespaces

#endif // BOOST_DECIMAL_CMATH_HPP
