//  Copyright (c) 2006 Xiaogang Zhang, 2015 John Maddock
//  Copyright 2024 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ELLINT_1_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ELLINT_1_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/cmath/impl/ellint_rf.hpp>
#include <boost/decimal/detail/cmath/impl/evaluate_polynomial.hpp>
#include <boost/decimal/detail/cmath/round.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/cmath/fmod.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>
#include <limits>
#include <type_traits>

namespace boost {
namespace decimal {
namespace detail {

/*
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ellint_k_imp(T k, const std::integral_constant<int, 0>&) -> T;
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ellint_k_imp(T k, const std::integral_constant<int, 1>&) -> T;
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ellint_k_imp(T k, const std::integral_constant<int, 2>&) -> T;

using precision_tag_type = std::integral_constant<int,
std::is_same<T, decimal32>::value ? 0 :
std::is_same<T, decimal64>::value ? 1 : 2>;
*/

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ellint_k_imp(T k) -> T
{
   if (abs(k) >= 1)
   {
      return std::numeric_limits<T>::signaling_NaN();
   }

   constexpr T x {0};
   T y {1 - k * k};
   constexpr T z {1};

   return ellint_impl::ellint_rf_imp(x, y, z);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ellint_f_impl(T phi, T k) noexcept -> T
{
    constexpr T half_pi {numbers::pi_v<T> / 2};

    bool invert {false};
    if (phi < 0)
    {
        phi = fabs(phi);
        invert = true;
    }

    T result {};

    if (isinf(phi))
    {
        return std::numeric_limits<T>::signaling_NaN();
    }

    if(phi > 1 / std::numeric_limits<T>::epsilon())
    {
        // Phi is so large that phi%pi is necessarily zero (or garbage),
        // just return the second part of the duplication formula:
        result = 2 * phi * ellint_k_imp(k) / numbers::pi_v<T>;
    }
    else
    {
        // Carlson's algorithm works only for |phi| <= pi/2,
        // use the integrand's periodicity to normalize phi
        //
        // Xiaogang's original code used a cast to long long here
        // but that fails if T has more digits than a long long,
        // so rewritten to use fmod instead:
        //
        T rphi = fmod(phi, half_pi);
        T m = round((phi - rphi) / half_pi);
        int s = 1;

        if (fmod(m, T{2}) > T{5, -1})
        {
            m += 1;
            s = -1;
            rphi = half_pi - rphi;
        }

        T sinp = sin(rphi);
        sinp *= sinp;

        if (sinp * k * k >= 1)
        {
            return std::numeric_limits<T>::signaling_NaN();
        }

        T cosp = cos(rphi);
        cosp *= cosp;
        if (sinp > (std::numeric_limits<T>::min)())
        {
            BOOST_DECIMAL_ASSERT(rphi != 0); // precondition, can't be true if sin(rphi) != 0.
            //
            // Use http://dlmf.nist.gov/19.25#E5, note that
            // c-1 simplifies to cot^2(rphi) which avoid cancellation:
            //
            T c = 1 / sinp;
            result = static_cast<T>(s * ellint_impl::ellint_rf_imp(cosp / sinp, c - k * k, c));
        }
        else
        {
            result = s * sin(rphi);
        }

        if(m != 0)
        {
            result += m * ellint_k_imp(k);
        }
    }

    return invert ? -result : result;
}

} //namespace detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto comp_ellint_1(T k) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
   return detail::ellint_k_imp(k);
}

/*
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T2>
constexpr auto ellint_1(T1 k, T2 phi) noexcept
   -> std::enable_if_t<detail::is_decimal_floating_point_v<T1> &&
                       detail::is_decimal_floating_point_v<T2>, detail::promote_args_t<T1, T2>>
{
   using promoted_type = detail::promote_args_t<T1, T2>;
   return detail::ellint_f_impl(static_cast<promoted_type>(phi), static_cast<promoted_type>(k));
}
*/

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ELLINT_1_HPP
