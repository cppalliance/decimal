// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_POW_HPP

#include <algorithm>
#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/pow_impl.hpp>
#include <boost/decimal/detail/type_traits.hpp>

namespace boost { namespace decimal {
template<typename T, typename IntegralType>
constexpr auto pow(T b, IntegralType p) noexcept -> std::enable_if_t<(detail::is_decimal_floating_point_v<T> && std::is_integral<IntegralType>::value), T> // NOLINT(misc-no-recursion)
{
    T result { };

    constexpr T one { 1, 0 };

    if  (p <  INT64_C(0)) { result = one / detail::pow_impl(b, static_cast<std::make_unsigned_t<IntegralType>>(-p)); }
    else                  { result =       detail::pow_impl(b, static_cast<std::make_unsigned_t<IntegralType>>( p)); }

    return result;
}

template<typename T>
constexpr auto pow(T x, T a) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    const auto fpc_x = fpclassify(x);
    const auto fpc_a = fpclassify(a);

    const auto na = static_cast<int>(a);

    const auto a_is_integer      = (na == a);
    const auto a_is_integer_even = (a_is_integer && ((na & 1) == 0));
    const auto a_is_integer_odd  = (a_is_integer && ((na & 1) != 0));

    constexpr T zero { 0, 0 };

    auto result = zero;

    // For non-normal arguments and special cases, see:
    //   https://en.cppreference.com/w/cpp/numeric/math/pow

    if (fpc_x == FP_ZERO)
    {
        if (fpc_a == FP_NORMAL)
        {
            const auto x_is_neg = signbit(x);

            if      ((!x_is_neg) && a_is_integer_odd && (na < 0))         { result =  std::numeric_limits<T>::infinity(); }
            else if (  x_is_neg  && a_is_integer_odd && (na < 0))         { result = -std::numeric_limits<T>::infinity(); }
            else if (  x_is_neg)                                          { result =  std::numeric_limits<T>::infinity(); }
            else if ((!x_is_neg) && a_is_integer_odd && (na > 0))         { result =  zero; }
            else if (  x_is_neg  && a_is_integer_odd && (na > 0))         { result = -zero; }
            else if ((!x_is_neg) && (a_is_integer_even || (!signbit(a)))) { result =  zero; }
        }
        else if (fpc_a == FP_INFINITE)
        {
            result = (signbit(a) ? std::numeric_limits<T>::infinity() : zero);
        }
    }
    else if (fpc_x == FP_INFINITE)
    {
        // TBD: Infinite |base| still needed.
        result = zero;
    }
    else if (fpc_x != FP_NORMAL)
    {
        result = x;
    }
    else
    {
        constexpr T one  { 1, 0 };

        using std::abs;

        const auto x_abs_is_one = (a_is_integer_odd && abs(na) == 1);

        if (x_abs_is_one)
        {
            if (((na < 0) && (fpc_a == FP_INFINITE)) || (na > 0)) { result = one; }
        }
        else if (fpc_a == FP_ZERO)
        {
            result = one;
        }
        else if (fpc_a == FP_INFINITE)
        {
            const auto abs_base = abs(x);

            if      (abs_base < one) { result = (signbit(a) ? std::numeric_limits<T>::infinity() : zero); }
            else if (abs_base > one) { result = (signbit(a) ? zero : std::numeric_limits<T>::infinity()); }
        }
        else if (fpc_a == FP_ZERO)
        {
            result = one;
        }
        else
        {
            const auto a_log_x = a * log(x);

            result = exp(a_log_x);
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
