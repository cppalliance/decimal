//  (C) Copyright John Maddock 2006.
//  (C) Copyright Matt Borland 2024.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ASSOC_LEGENDRE_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ASSOC_LEGENDRE_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/cmath/log1p.hpp>
#include <boost/decimal/detail/cmath/tgamma.hpp>
#include <boost/decimal/detail/cmath/pow.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/legendre.hpp>
#include <utility>
#include <type_traits>
#include <limits>

namespace boost {
namespace decimal {

namespace detail {

// TODO(mborland): make p0 lookup the same as sin, cos, etc.

template <typename T1, typename T2, typename T3>
constexpr auto assoc_legendre_next(unsigned l, unsigned m, T1 x, T2 Pl, T3 Plm1) noexcept
{
    using result_type = promote_args_t<T1, T2, T3>;
    return ((2 * l + 1) * static_cast<result_type>(x) * static_cast<result_type>(Pl) - (l + m) * static_cast<result_type>(Plm1)) / (l + 1 - m);
}

// Implement Legendre P and Q polynomials via recurrence:
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr T assoc_legendre_impl(unsigned l, unsigned m, T x, T sin_theta_power) noexcept
{
    if (x < -1 || x > 1 || l > 128)
    {
        return std::numeric_limits<T>::signaling_NaN();
    }
    else if (isnan(x))
    {
        return x;
    }

    if (l == 0 && m == -1)
    {
        return sqrt((1 - x) / (1 + x));
    }
    else if (l == 1 && m == 0)
    {
        return x;
    }
    else if (m > l)
    {
        return T{0};
    }
    else if (m == 0)
    {
        return legendre(l, x);
    }

    BOOST_DECIMAL_ASSERT_MSG(m <= 50, "m > 50 has not been implemented");
    T p0 = p0_lookup<T>(2 * m - 1) * sin_theta_power;

    if (m & 1)
    {
        p0 = -p0;
    }

    if (m == l)
    {
        return p0;
    }

    T p1 = x * (2 * m + 1) * p0;

    auto n = m + 1;

    while (n < l)
    {
        std::swap(p0, p1);
        p1 = assoc_legendre_next(n, m, x, p0, p1);
        ++n;
    }

    return p1;
}

} //namespace detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto assoc_legendre(unsigned n, unsigned m, T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    return detail::assoc_legendre_impl(n, m, x, pow(1 - x*x, T{m} / 2));
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ASSOC_LEGENDRE_HPP
