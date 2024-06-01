// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_IMPL_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/concepts.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstddef>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace ellint_detail {

namespace elliptic_series {

template <typename T>
constexpr auto ellint_decimal_order(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    int n { };

    static_cast<void>(frexp10(x, &n));

    constexpr int order_bias
    {
          std::numeric_limits<T>::digits10 < 10 ?  6
        : std::numeric_limits<T>::digits10 < 20 ? 15
        :                                         33
    };

    return n + order_bias;
}

template <typename T>
constexpr auto agm(T  phi,
                   T  mk,
                   T& Fpm,
                   T& Km,
                   T* const pEm  = nullptr,
                   T* const pEpm = nullptr) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, void)
{
  // Use the AGM algorithm as described in Computation of Special Functions,
  // Zhang & Jin, 18.3.2, pages 663-665. The implementation is based on the
  // sample code therein. However, the Mathematica argument convention with
  // (k^2 --> m) is used, as described in Stephen Wolfram's Mathematica Book,
  // 4th Ed., Ch. 3.2.11, Page 773.

  // Make use of the following properties:
  // F(z + pi*j | m) = F(z | m) + 2j pi K(m)
  // E(z + pi*j | m) = E(z | m) + 2j pi E(m)

  // as well as:
  // F(-z | m) = -F(z | m)
  // E(-z | m) = -E(z | m)

  // The calculations which are needed for EllipticE(...) are only performed if
  // the results from these will actually be used, in other words only if non-zero
  // pointers pEm or pEpm have been supplied to this subroutine.

  // Note that there is special handling for the angular argument phi if this
  // argument is equal to pi/2.

  auto fpc_m = fpclassify(mk);

  constexpr T my_pi_half { numbers::pi_v<T> / 2 };

  const bool phi_is_pi_half = (phi == my_pi_half);

  T zero { 0 };
  T one  { 1 };

  if(fpc_m == FP_ZERO)
  {
    Fpm = phi;
    Km  = my_pi_half;

    if(pEpm != nullptr) { *pEpm = phi; }
    if(pEm  != nullptr) { *pEm  = my_pi_half; }
  }
  else if(mk == one)
  {
    if(pEm != nullptr) { *pEm = one; }

    Km = std::numeric_limits<T>::quiet_NaN();

    const T sp = sin(phi);

    Fpm = phi_is_pi_half ? std::numeric_limits<T>::quiet_NaN()
                         : log((one + sp) / (one - sp)) / 2;

    if(pEpm != nullptr) { *pEpm = phi_is_pi_half ? one : sp; }
  }
  else
  {
    constexpr T half { 5 , -1 };

    T a0    = one;
    T b0    = sqrt(one - mk * mk);
    T phi_n = phi;

    std::uint32_t p2 { UINT32_C(1) };

    T an { };

    const bool has_e { ((pEm  != nullptr) || (pEpm != nullptr)) };

    T cn_2ncn_inner_prod      = (has_e ? (mk * mk) / 2 : zero);
    T sin_phi_n_cn_inner_prod = zero;

    for(int n = 1; n < std::numeric_limits<std::uint32_t>::digits; ++n)
    {
      an = (a0 + b0) / 2;

      if(!phi_is_pi_half) { phi_n += atan((b0 / a0) * tan(phi_n)); }

      const T cn_term = (a0 - b0) / 2;

      if(has_e)
      {
        cn_2ncn_inner_prod += ((cn_term * cn_term) * p2);

        if(pEpm != nullptr)
        {
          const T spn_term = ((!phi_is_pi_half) ? sin(phi_n) : zero);

          sin_phi_n_cn_inner_prod += (cn_term * spn_term);
        }
      }

      p2 = static_cast<std::uint32_t>(p2 << 1U);

      constexpr T near_one { 9 , -1 };

      // TODO(ckormanyos) There should be a better way to formulate the break condition.
      // And the condition should depend on some actual values of iteration terms in
      // relation to the decimal digits of the type.

      // TODO(ckormanyos) Use a Taylor series or similar approx. for phi near 0.
      // TODO(ckormanyos) Use a Taylor series or similar approx. for m near 1.

      const int iter_low_limit = (mk < half) ? 2 : (mk < near_one) ? 3 : 4;

      if(n > iter_low_limit)
      {
        break;
      }

      b0 = sqrt(a0 * b0);
      a0 = an;

      if(!phi_is_pi_half)
      {
        phi_n += numbers::pi_v<T> * static_cast<int>((phi_n / numbers::pi_v<T>) + half);
      }
    }

    const T one_over_an = one / an;

    Fpm = phi_n * one_over_an;

    if(!phi_is_pi_half) { Fpm /= p2; }

    Km = my_pi_half * one_over_an;

    if(has_e)
    {
      const T one_minus_cn_2ncn_inner_prod_half = one - cn_2ncn_inner_prod;

      if(pEm != nullptr)
      {
        *pEm = Km * one_minus_cn_2ncn_inner_prod_half;
      }

      if(pEpm != nullptr)
      {
        *pEpm = (Fpm * one_minus_cn_2ncn_inner_prod_half) +  sin_phi_n_cn_inner_prod;
      }
    }
  }
}

} // namespace elliptic_series

} //namespace ellint_detail


} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_IMPL_HPP
