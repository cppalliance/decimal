// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CMATH_HPP
#define BOOST_DECIMAL_CMATH_HPP

#include <boost/decimal/detail/promotion.hpp>
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
#include <boost/decimal/detail/cmath/log.hpp>
#include <boost/decimal/detail/cmath/log10.hpp>
#include <boost/decimal/detail/cmath/pow.hpp>
#include <boost/decimal/detail/cmath/remainder.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/sin.hpp>
#include <boost/decimal/detail/cmath/trunc.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <boost/decimal/detail/cmath/modf.hpp>
#include <boost/decimal/detail/cmath/fmod.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/hypot.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

// Generic overloads

constexpr auto frexp10(decimal32 num, int *expptr) noexcept -> std::uint32_t
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

constexpr auto copysign(decimal32 mag, decimal32 sgn) noexcept -> decimal32
{
    return copysignd32(mag, sgn);
}

constexpr auto fma(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return fmad32(x, y, z);
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
