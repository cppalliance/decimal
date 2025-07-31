// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ASIN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ASIN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/impl/asin_impl.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto asin_impl(const T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    const auto fpc {fpclassify(x)};
    const auto isneg {signbit(x)};

    if (fpc == FP_ZERO
        #ifndef BOOST_DECIMAL_FAST_MATH
        || fpc == FP_NAN
        #endif
        )
    {
        return x;
    }

    const auto absx {fabs(x)};
    T result {};

    if (absx <= std::numeric_limits<T>::epsilon())
    {
        result = absx;
    }
    else if (absx <= T{5, -1})
    {
        result = asin_series(absx);
    }
    else if (absx <= T{1, 0})
    {
        constexpr T half_pi {numbers::pi_v<T> / 2};
        result = half_pi - 2 * asin_series(sqrt((1 - absx) / 2));
    }
    else
    {
        #ifndef BOOST_DECIMAL_FAST_MATH
        result = std::numeric_limits<T>::quiet_NaN();
        #else
        result = T{0};
        #endif
    }

    // arcsin(-x) == -arcsin(x)
    if (isneg)
    {
        result = -result;
    }

    return result;
}

} //namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto asin(const T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64_t>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128_t>;

    #endif

    return static_cast<T>(detail::asin_impl(static_cast<evaluation_type>(x)));
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ASIN_HPP
