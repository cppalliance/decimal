// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CMATH_HPP
#define BOOST_DECIMAL_CMATH_HPP

// TODO(mborland): Allow conversion between decimal types via a promotion system
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/cmath/fdim.hpp>
#include <boost/decimal/detail/cmath/fmax.hpp>
#include <boost/decimal/detail/cmath/fmin.hpp>
#include <boost/decimal/detail/cmath/frexp.hpp>
#include <boost/decimal/detail/cmath/ilogb.hpp>
#include <boost/decimal/detail/cmath/isgreater.hpp>
#include <boost/decimal/detail/cmath/isless.hpp>
#include <boost/decimal/detail/cmath/isunordered.hpp>
#include <boost/decimal/detail/cmath/ldexp.hpp>
#include <boost/decimal/detail/cmath/pow.hpp>
#include <boost/decimal/detail/cmath/remainder.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/sin.hpp>
#include <boost/decimal/detail/cmath/trunc.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/numbers.hpp>

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

constexpr auto frexp10(decimal32 num, int *expptr) noexcept -> std::int32_t
{
    return frexp10d32(num, expptr);
}

constexpr auto scalbn(decimal32 num, int expval) noexcept -> decimal32
{
    return scalbnd32(num, expval);
}

constexpr auto scalbln(decimal32 num, long expval) noexcept -> decimal32
{
    return scalblnd32(num, expval);
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

constexpr auto modf(decimal32 x, decimal32* iptr) noexcept -> decimal32
{
    return modfd32(x, iptr);
}

constexpr auto samequantum(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return samequantumd32(lhs, rhs);
}

constexpr auto quantexp(decimal32 x) noexcept -> int
{
    return quantexpd32(x);
}

constexpr auto quantize(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return quantized32(lhs, rhs);
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_CMATH_HPP
