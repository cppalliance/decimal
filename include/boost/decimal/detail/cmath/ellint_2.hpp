//  Copyright 2002 2011, 2024 Christopher Kormanyos
//  Copyright 2024 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ELLINT_2_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ELLINT_2_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/ellint_impl.hpp>
#include <boost/decimal/detail/cmath/atan.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/cmath/log.hpp>
#include <boost/decimal/detail/cmath/sin.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/tan.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/numbers.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <type_traits>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto ellint_2_impl(T m, T phi) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
  constexpr T one { 1 };

  T result { };

  const auto fpc_m   = fpclassify(m);
  const auto fpc_phi = fpclassify(phi);

  if((fpc_m == FP_ZERO) && (fpc_phi == FP_NORMAL))
  {
    result = phi;
  }
  else if((fpc_phi == FP_ZERO) && (fpc_m == FP_NORMAL))
  {
    constexpr T zero { 0 };

    result = zero;
  }
  else if((fabs(m) > one) || (fpc_phi != FP_NORMAL) || (fpc_m != FP_NORMAL))
  {
    result = std::numeric_limits<T>::quiet_NaN();
  }
  else if(signbit(phi))
  {
    result = -ellint_2_impl(m, -phi);
  }
  else if(signbit(m))
  {
    result = ellint_2_impl(-m, phi);
  }
  else
  {
    constexpr int small_phi_order
    {
          std::numeric_limits<T>::digits10 < 10 ? 2
        : std::numeric_limits<T>::digits10 < 20 ? 4
        :                                         8
    };

    if (phi < T { 1, -small_phi_order })
    {
      // PadeApproximant[EllipticE[phi, m2], {phi, 0, {4, 3}}]
      // FullSimplify[%]
      // Then manually edit the interior field to regain HornerForm[poly, phi].

      const T phi_sq { phi * phi };

      const T m2 { (!signbit(m)) ? (m * m) : -(m * m) };

      const T top { phi * (-60 + (-12 + 19 * m2) * phi_sq) };
      const T bot { -60 + 3 * (-4 + 3 * m2) * phi_sq };

      result = top / bot;
    }
    else
    {
      constexpr T my_pi_half { numbers::pi_v<T> / 2 };

      T k_pi       = static_cast<int>(phi / numbers::pi_v<T>);
      T phi_scaled = phi - (k_pi * numbers::pi_v<T>);

      const bool b_neg { phi_scaled > my_pi_half };

      if(b_neg)
      {
        ++k_pi;

        phi_scaled = -(phi_scaled - numbers::pi_v<T>);
      }

      T Fpm { };
      T Km  { };
      T Em  { };

      detail::ellint_detail::elliptic_series::agm(phi_scaled, m, Fpm, Km, &Em, &result);

      if(b_neg)
      {
        result = -result;
      }

      result += ((k_pi * Em) * 2);
    }
  }

  return result;
}

template <typename T>
constexpr auto comp_ellint_2_impl(T m) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
  constexpr T one  { 1 };

  T result { };

  const auto fpc_m   = fpclassify(m);

  if(fpc_m == FP_ZERO)
  {
    result = numbers::pi_v<T> / 2;
  }
  else if((fabs(m) > one) || (fpc_m != FP_NORMAL))
  {
    result = std::numeric_limits<T>::quiet_NaN();
  }
  else if(signbit(m))
  {
    result = comp_ellint_2_impl(-m);
  }
  else
  {
    constexpr T zero { 0 };

    T Fpm { };
    T Km  { };

    detail::ellint_detail::elliptic_series::agm(zero, m, Fpm, Km, &result);
  }

  return result;
}

} //namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto ellint_2(T k, T phi) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::ellint_2_impl(static_cast<evaluation_type>(k), static_cast<evaluation_type>(phi)));
}

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto comp_ellint_2(T k) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::comp_ellint_2_impl(static_cast<evaluation_type>(k)));
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ELLINT_2_HPP
