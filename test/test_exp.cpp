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

template<typename DecimalType> auto my_zero() -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_zero { 0, 0 }; return val_zero; }
template<typename DecimalType> auto my_one () -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_one  { 1, 0 }; return val_one; }

namespace local
{
  template<typename IntegralTimePointType,
           typename ClockType = std::chrono::high_resolution_clock>
  auto time_point() noexcept -> IntegralTimePointType
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

  template<typename NumericType>
  auto is_close_fraction(const NumericType& a,
                         const NumericType& b,
                         const NumericType& tol) noexcept -> bool
  {
    using std::fabs;

    auto result_is_ok = bool { };

    if(b == static_cast<NumericType>(0))
    {
      result_is_ok = (fabs(a - b) < tol);
    }
    else
    {
      const auto delta = fabs(1 - (a / b));

      result_is_ok = (delta < tol);
    }

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_exp(const int tol_factor, const bool negate, const long double range_lo, const long double range_hi) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::random_device rd;
    std::mt19937_64 gen(rd());

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    auto dis =
      std::uniform_real_distribution<float_type>
      {
        static_cast<float_type>(range_lo),
        static_cast<float_type>(range_hi)
      };

    auto result_is_ok = true;

    auto trials = static_cast<std::uint32_t>(UINT8_C(0));

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    constexpr auto count = (sizeof(decimal_type) == static_cast<std::size_t>(UINT8_C(4))) ? static_cast<std::uint32_t>(UINT32_C(0x400)) : static_cast<std::uint32_t>(UINT32_C(0x40));
    #else
    constexpr auto count = (sizeof(decimal_type) == static_cast<std::size_t>(UINT8_C(4))) ? static_cast<std::uint32_t>(UINT32_C(0x40)) : static_cast<std::uint32_t>(UINT32_C(0x4));
    #endif

    for( ; trials < count; ++trials)
    {
      const auto x_flt_begin = dis(gen);

      const auto x_flt = (negate ? -x_flt_begin : x_flt_begin);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::exp;

      const auto val_flt = exp(x_flt);
      const auto val_dec = exp(x_dec);

      const auto result_val_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), static_cast<float_type>(std::numeric_limits<decimal_type>::epsilon()) * static_cast<float_type>(tol_factor));

      result_is_ok = (result_val_is_ok && result_is_ok);

      if(!result_val_is_ok)
      {
          // LCOV_EXCL_START
        std::cout << "x_flt  : " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << x_flt   << std::endl;
        std::cout << "val_flt: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_flt << std::endl;
        std::cout << "val_dec: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_dec << std::endl;

        break;
          // LCOV_EXCL_STOP
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_exp_edge() -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::mt19937_64 gen;

    std::uniform_real_distribution<float_type>
      dist
      (
        static_cast<float_type>(1.01L),
        static_cast<float_type>(1.04L)
      );

    auto result_is_ok = true;

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_nan = exp(std::numeric_limits<decimal_type>::quiet_NaN() * static_cast<decimal_type>(dist(gen)));

      const auto result_val_nan_is_ok = isnan(val_nan);

      BOOST_TEST(result_val_nan_is_ok);

      result_is_ok = (result_val_nan_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_inf_pos = exp(std::numeric_limits<decimal_type>::infinity() * static_cast<decimal_type>(dist(gen)));

      const auto result_val_inf_pos_is_ok = isinf(val_inf_pos);

      BOOST_TEST(result_val_inf_pos_is_ok);

      result_is_ok = (result_val_inf_pos_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_inf_neg = exp(-std::numeric_limits<decimal_type>::infinity() * static_cast<decimal_type>(dist(gen)));

      const auto result_val_inf_neg_is_ok = (val_inf_neg == ::my_zero<decimal_type>());

      BOOST_TEST(result_val_inf_neg_is_ok);

      result_is_ok = (result_val_inf_neg_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_zero_pos = exp(::my_zero<decimal_type>());

      const auto result_val_zero_pos_is_ok = (val_zero_pos == ::my_one<decimal_type>());

      BOOST_TEST(result_val_zero_pos_is_ok);

      result_is_ok = (result_val_zero_pos_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_zero_neg = exp(-::my_zero<decimal_type>());

      const auto result_val_zero_neg_is_ok = (val_zero_neg == ::my_one<decimal_type>());

      BOOST_TEST(result_val_zero_neg_is_ok);

      result_is_ok = (result_val_zero_neg_is_ok && result_is_ok);
    }

    return result_is_ok;
  }
} // namespace local

auto main() -> int
{
  auto result_is_ok = true;

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto result_pos_is_ok = local::test_exp<decimal_type, float_type>(128, false, 0.03125L, 80.0L);
    const auto result_neg_is_ok = local::test_exp<decimal_type, float_type>(128, true,  0.03125L, 80.0L);

    const auto result_pos_narrow_is_ok = local::test_exp<decimal_type, float_type>(64, false, 0.25L, 4.0L);
    const auto result_neg_narrow_is_ok = local::test_exp<decimal_type, float_type>(64, true,  0.25L, 4.0L);

    const auto result_edge_is_ok = local::test_exp_edge<decimal_type, float_type>();

    BOOST_TEST(result_pos_is_ok);
    BOOST_TEST(result_neg_is_ok);

    BOOST_TEST(result_pos_narrow_is_ok);
    BOOST_TEST(result_neg_narrow_is_ok);

    BOOST_TEST(result_edge_is_ok);

    result_is_ok = (result_pos_is_ok && result_is_ok);
    result_is_ok = (result_neg_is_ok && result_is_ok);

    result_is_ok = (result_pos_narrow_is_ok && result_is_ok);
    result_is_ok = (result_neg_narrow_is_ok && result_is_ok);

    result_is_ok = (result_edge_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto result_pos_lo_is_ok = local::test_exp<decimal_type, float_type>(512, false, 0.03125L, 80.0L);
    const auto result_neg_lo_is_ok = local::test_exp<decimal_type, float_type>(512, true,  0.03125L, 80.0L);

    const auto result_pos_hi_is_ok = local::test_exp<decimal_type, float_type>(3072, false, 8.0L, 512.0L);
    const auto result_neg_hi_is_ok = local::test_exp<decimal_type, float_type>(3072, true,  8.0L, 512.0L);

    const auto result_edge_is_ok = local::test_exp_edge<decimal_type, float_type>();

    BOOST_TEST(result_pos_lo_is_ok);
    BOOST_TEST(result_neg_lo_is_ok);

    BOOST_TEST(result_pos_hi_is_ok);
    BOOST_TEST(result_neg_hi_is_ok);

    BOOST_TEST(result_edge_is_ok);

    result_is_ok = (result_pos_lo_is_ok && result_is_ok);
    result_is_ok = (result_neg_lo_is_ok && result_is_ok);

    result_is_ok = (result_pos_hi_is_ok && result_is_ok);
    result_is_ok = (result_neg_hi_is_ok && result_is_ok);

    result_is_ok = (result_edge_is_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
