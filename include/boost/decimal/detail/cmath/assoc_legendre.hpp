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

// Long double precision double factorials
static constexpr std::array<long double, 48> p0_values = {{
    1.0L,
    1.0L,
    2.0L,
    3.0L,
    8.0L,
    15.0L,
    48.0L,
    105.0L,
    384.0L,
    945.0L,
    3840.0L,
    10395.0L,
    46080.0L,
    135135.0L,
    645120.0L,
    2027025.0L,
    10321920.0L,
    34459425.0L,
    185794560.0L,
    654729075.0L,
    3715891200.0L,
    13749310575.0L,
    81749606400.0L,
    316234143225.0L,
    1961990553600.0L,
    7905853580625.0L,
    51011754393600.0L,
    213458046676875.0L,
    1428329123020800.0L,
    6190283353629375.0L,
    42849873690624000.0L,
    191898783962510625.0L,
    1371195958099968000.0L,
    6332659870762850625.0L,
    46620662575398912000.0L,
    221643095476699771872.0L,
    1678343852714360832000.0L,
    8200794532637891558912.0L,
    63777066403145711616000.0L,
    319830986772877770817536.0L,
    2551082656125828464640000.0L,
    13113070457687988603191296.0L,
    107145471557284795514880000.0L,
    563862029680583509939322880.0L,
    4714400748520531002654720000.0L,
    25373791335626257946766213120.0L,
    216862434431944426122117120000.0L,
    1.19256819277443412350660195123e+30L
}};

template <typename T>
constexpr T p0_lookup(unsigned i) noexcept
{
    return static_cast<T>(p0_values[i]);
}

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

    BOOST_DECIMAL_ASSERT_MSG(m <= 24, "m > 25 has not been implemented");
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
