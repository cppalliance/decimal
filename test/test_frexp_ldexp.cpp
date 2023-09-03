// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using decimal_type = boost::decimal::decimal32;

namespace local
{
  template<typename IntegralTimePointType,
           typename ClockType = std::chrono::high_resolution_clock>
  auto time_point() -> IntegralTimePointType
  {
    using local_integral_time_point_type = IntegralTimePointType;
    using local_clock_type               = ClockType;

    const auto current_now =
      static_cast<std::uintmax_t>
      (
        std::chrono::duration_cast<std::chrono::nanoseconds>
        (
          local_clock_type::now().time_since_epoch()
        ).count()
      );

    return static_cast<local_integral_time_point_type>(current_now);
  }

  template<typename FloatType>
  auto is_excluded(const FloatType x) -> bool
  {
    return ((x >= static_cast<FloatType>(8388606.5L)) && (x < static_cast<FloatType>(8388607.5L)));
  }

  template<typename NumericType>
  auto is_close_fraction(const NumericType& a,
                         const NumericType& b,
                         const NumericType& tol) -> bool
  {
    using std::fabs;

    const auto delta = fabs(1 - fabs(a / b));

    return (delta < tol);
  }

  struct test_frexp_ldexp_ctrl
  {
    constexpr test_frexp_ldexp_ctrl(const float lo, const float hi, const bool b, const std::uint32_t c)
      : float_value_lo(lo),
        float_value_hi(hi),
        b_neg         (b),
        count         (c) { }

    const float         float_value_lo;
    const float         float_value_hi;
    const bool          b_neg;
    const std::uint32_t count;
  };

  auto test_frexp_ldexp(const test_frexp_ldexp_ctrl& ctrl) -> bool
  {
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937_64 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

    std::uniform_real_distribution<float> dis(ctrl.float_value_lo, ctrl.float_value_hi);

    auto result_is_ok = true;

    auto trials = static_cast<std::uint32_t>(UINT8_C(0));

    for( ; trials < ctrl.count; ++trials)
    {
      const auto trials_mod_64k =
        static_cast<std::uint32_t>
        (
          static_cast<std::uint32_t>(trials + static_cast<std::uint32_t>(UINT8_C(1))) % static_cast<std::uint32_t>(UINT32_C(0x10000))
        );

      if(trials_mod_64k == static_cast<std::uint32_t>(UINT8_C(0)))
      {
        const auto sd = time_point<std::uint64_t>();

        gen.seed(static_cast<typename std::mt19937_64::result_type>(sd));
      }

      auto flt_start = float { };

      for(;;)
      {
        flt_start = dis(gen);

        if(flt_start != static_cast<float>(0.0L) && (!is_excluded(flt_start)))
        {
          break;
        }
      }

      if(ctrl.b_neg) { flt_start = -flt_start; }

      const auto dec = static_cast<decimal_type>(flt_start);
      const auto flt = static_cast<float>(dec);

      using std::frexp;

      int n_flt;
      const auto frexp_flt = frexp(flt, &n_flt);

      int n_dec;
      const auto frexp_dec = frexp(dec, &n_dec);

      using std::ldexp;

      const auto ldexp_flt = ldexp(frexp_flt, n_flt);
      const auto ldexp_dec = ldexp(frexp_dec, n_dec);

      const auto ldexp_dec_as_float = static_cast<float>(ldexp_dec);

      const auto tol_n =
        static_cast<float>
        (
            static_cast<float>(std::numeric_limits<decimal_type>::epsilon())
          * static_cast<float>(16)
        );

      const auto result_frexp_ldexp_is_ok = is_close_fraction(ldexp_flt, ldexp_dec_as_float, tol_n);

      if(!result_frexp_ldexp_is_ok)
      {
        break;
      }

      result_is_ok = (result_frexp_ldexp_is_ok && result_is_ok);
    }

    result_is_ok = ((trials == ctrl.count) && result_is_ok);

    return result_is_ok;
  }
}

auto main() -> int
{
  constexpr auto flt_ctrl =
    std::array<local::test_frexp_ldexp_ctrl, static_cast<std::size_t>(UINT8_C(5))>
    {
      local::test_frexp_ldexp_ctrl( { -1.0E7F  , +1.0E7F,  false, static_cast<std::uint32_t>(UINT32_C(0x10000)) } ),
      local::test_frexp_ldexp_ctrl( {  1.0E-20F, +1.0E-1F, false, static_cast<std::uint32_t>(UINT32_C(0x10000)) } ),
      local::test_frexp_ldexp_ctrl( {  1.0E-20F, +1.0E-1F, true , static_cast<std::uint32_t>(UINT32_C(0x10000)) } ),
      local::test_frexp_ldexp_ctrl( {  10.0F   , +1.0E12F, false, static_cast<std::uint32_t>(UINT32_C(0x10000)) } ),
      local::test_frexp_ldexp_ctrl( {  10.0F   , +1.0E12F, true , static_cast<std::uint32_t>(UINT32_C(0x10000)) } ),
    };

  auto result_is_ok = true;

  for(const auto& ctrl : flt_ctrl)
  {
    const auto result_test_frexp_ldexp_is_ok = local::test_frexp_ldexp(ctrl);

    BOOST_TEST(result_test_frexp_ldexp_is_ok);

    result_is_ok = (result_test_frexp_ldexp_is_ok && result_is_ok);

    std::cout << "result_test_frexp_ldexp_is_ok: " << std::boolalpha << result_test_frexp_ldexp_is_ok << std::endl;
  }

  std::cout << "result_is_ok                 : " << std::boolalpha << result_is_ok << std::endl;

  return boost::report_errors();
}
