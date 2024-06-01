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

  if(fabs(m) > one)
  {
    result = std::numeric_limits<T>::quiet_NaN();
  }
  else
  {
    if(signbit(phi))
    {
      result = -ellint_2(m, -phi);
    }
    else
    {
      constexpr int small_phi_order
      {
            std::numeric_limits<T>::digits10 < 10 ?  3
          : std::numeric_limits<T>::digits10 < 20 ?  5
          :                                         10
      };

      if (phi < T { 5, -small_phi_order })
      {
        // Normal[Series[EllipticE[phi, m m], {phi, 0, 8}]]
        // Together[%]
        // Then manually edit the interior field to regain HornersForm[poly, phi].

        const T phi_sq { phi * phi };

        const T msq { m * m };

        const T k   { signbit(m) ? -msq : msq };
        const T ksq { k * k };
        const T kcb { k * ksq };

        result = (phi * (5040 + phi_sq * (-840 * k + phi_sq * (168 * k - 126 * ksq + phi_sq * (-16 * k + 60 * ksq - 45 * kcb))))) / 5040;
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
        T Epm { };

        detail::ellint_detail::elliptic_series::agm(phi_scaled, m, Fpm, Km, &Em, &Epm);

        if(b_neg)
        {
          Epm = -Epm;
        }

        result = Epm + ((k_pi * Em) * 2);
      }
    }
  }

  return result;
}

template <typename T>
constexpr auto comp_ellint_2_impl(T m) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
  constexpr T one { 1 };

  if(fabs(m) > one)
  {
    return std::numeric_limits<T>::quiet_NaN();
  }
  else
  {
    T Fpm { };
    T Km  { };
    T Em  { };

    constexpr T zero { 0 };

    detail::ellint_detail::elliptic_series::agm(zero, m, Fpm, Km, &Em);

    return Em;
  }
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
