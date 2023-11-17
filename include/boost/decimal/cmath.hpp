// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CMATH_HPP
#define BOOST_DECIMAL_CMATH_HPP

#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/acosh.hpp>
#include <boost/decimal/detail/cmath/asinh.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/cosh.hpp>
#include <boost/decimal/detail/cmath/exp.hpp>
#include <boost/decimal/detail/cmath/exp2.hpp>
#include <boost/decimal/detail/cmath/expm1.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/cmath/fdim.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/fmax.hpp>
#include <boost/decimal/detail/cmath/fmod.hpp>
#include <boost/decimal/detail/cmath/fmin.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/frexp.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>
#include <boost/decimal/detail/cmath/hypot.hpp>
#include <boost/decimal/detail/cmath/ilogb.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/isgreater.hpp>
#include <boost/decimal/detail/cmath/isless.hpp>
#include <boost/decimal/detail/cmath/isunordered.hpp>
#include <boost/decimal/detail/cmath/ldexp.hpp>
#include <boost/decimal/detail/cmath/log.hpp>
#include <boost/decimal/detail/cmath/log1p.hpp>
#include <boost/decimal/detail/cmath/log10.hpp>
#include <boost/decimal/detail/cmath/modf.hpp>
#include <boost/decimal/detail/cmath/nearbyint.hpp>
#include <boost/decimal/detail/cmath/next.hpp>
#include <boost/decimal/detail/cmath/pow.hpp>
#include <boost/decimal/detail/cmath/remainder.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/rint.hpp>
#include <boost/decimal/detail/cmath/round.hpp>
#include <boost/decimal/detail/cmath/sin.hpp>
#include <boost/decimal/detail/cmath/sinh.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/tan.hpp>
#include <boost/decimal/detail/cmath/tanh.hpp>
#include <boost/decimal/detail/cmath/tgamma.hpp>
#include <boost/decimal/detail/cmath/trunc.hpp>
#include <boost/decimal/numbers.hpp>

// Macros from 3.6.2
#define BOOST_DECIMAL_HUGE_VAL_D32 std::numeric_limits<boost::decimal::decimal32>::infinity()
#define BOOST_DECIMAL_HUGE_VAL_D64 std::numeric_limits<boost::decimal::decimal64>::infinity()
#define BOOST_DECIMAL_DEC_INFINITY std::numeric_limits<boost::decimal::decimal64>::infinity()
#define BOOST_DECIMAL_DEC_NAN std::numeric_limits<boost::decimal::decimal64>::signaling_NaN()
#define BOOST_DECIMAL_FP_FAST_FMAD32 1
#define BOOST_DECIMAL_FP_FAST_FMAD64 1

namespace boost { namespace decimal {

constexpr auto scalbn(decimal32 num, int expval) noexcept -> decimal32
{
    return scalbnd32(num, expval);
}

constexpr auto scalbn(decimal64 num, int expval) noexcept -> decimal64
{
    return scalbnd64(num, expval);
}

constexpr auto scalbn(decimal128 num, int expval) noexcept -> decimal128
{
    return scalbnd128(num, expval);
}

constexpr auto scalbln(decimal32 num, long expval) noexcept -> decimal32
{
    return scalblnd32(num, expval);
}

constexpr auto scalbln(decimal64 num, long expval) noexcept -> decimal64
{
    return scalblnd64(num, expval);
}

constexpr auto scalbln(decimal128 num, long expval) noexcept -> decimal128
{
    return scalblnd128(num, expval);
}

constexpr auto copysign(decimal32 mag, decimal32 sgn) noexcept -> decimal32
{
    return copysignd32(mag, sgn);
}

constexpr auto copysign(decimal64 mag, decimal64 sgn) noexcept -> decimal64
{
    return copysignd64(mag, sgn);
}

constexpr auto copysign(decimal128 mag, decimal128 sgn) noexcept -> decimal128
{
    return copysignd128(mag, sgn);
}

constexpr auto fma(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return fmad32(x, y, z);
}

constexpr auto fma(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64
{
    return fmad64(x, y, z);
}

constexpr auto samequantum(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return samequantumd32(lhs, rhs);
}

constexpr auto samequantum(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    return samequantumd64(lhs, rhs);
}

constexpr auto samequantum(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    return samequantumd128(lhs, rhs);
}

constexpr auto quantexp(decimal32 x) noexcept -> int
{
    return quantexpd32(x);
}

constexpr auto quantexp(decimal64 x) noexcept -> int
{
    return quantexpd64(x);
}

constexpr auto quantexp(decimal128 x) noexcept -> int
{
    return quantexpd128(x);
}

constexpr auto quantize(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return quantized32(lhs, rhs);
}

constexpr auto quantize(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return quantized64(lhs, rhs);
}

constexpr auto quantize(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    return quantized128(lhs, rhs);
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_CMATH_HPP
