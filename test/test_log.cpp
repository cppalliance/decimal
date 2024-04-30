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

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/core/lightweight_test.hpp>

template<typename DecimalType> auto my_zero() -> DecimalType& { using decimal_type = DecimalType; static decimal_type my_zero { 0, 0 }; return my_zero; }
template<typename DecimalType> auto my_one () -> DecimalType& { using decimal_type = DecimalType; static decimal_type my_one  { 1, 0 }; return my_one; }
template<typename DecimalType> auto my_inf () -> DecimalType& { using decimal_type = DecimalType; static decimal_type my_inf  { std::numeric_limits<decimal_type>::infinity() };  return my_inf; }
template<typename DecimalType> auto my_nan () -> DecimalType& { using decimal_type = DecimalType; static decimal_type my_nan  { std::numeric_limits<decimal_type>::quiet_NaN() }; return my_nan; }

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
  auto test_log(const int tol_factor) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::random_device rd;
    std::mt19937_64 gen(rd());

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    auto dis =
      std::uniform_real_distribution<float_type>
      {
        static_cast<float_type>(1.0E-17L),
        static_cast<float_type>(1.0E+17L)
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
      const auto x_flt = dis(gen);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::log;

      const auto val_flt = log(x_flt);
      const auto val_dec = log(x_dec);

      const auto result_log_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), std::numeric_limits<float_type>::epsilon() * static_cast<float_type>(tol_factor));

      result_is_ok = (result_log_is_ok && result_is_ok);

      if(!result_log_is_ok)
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
  auto test_log_between_1_and_2(const int tol_factor) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    auto result_is_ok = true;

    for(auto   ui_arg = static_cast<unsigned>(UINT8_C(106));
               ui_arg < static_cast<unsigned>(UINT8_C(205));
             ++ui_arg)
    {
      const auto x_dec = static_cast<decimal_type>(ui_arg) / 100U;
      const auto x_flt = static_cast<float_type>(x_dec);

      using std::log;

      const auto val_flt = log(x_flt);
      const auto val_dec = log(x_dec);

      const auto result_log_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), std::numeric_limits<float_type>::epsilon() * static_cast<float_type>(tol_factor));

      result_is_ok = (result_log_is_ok && result_is_ok);

      if(!result_log_is_ok)
      {
          // LCOV_EXCL_START
        std::cout << "x_flt  : " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << x_flt   << std::endl;
        std::cout << "val_flt: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_flt << std::endl;
        std::cout << "val_dec: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_dec << std::endl;

        break;
          // LCOV_EXCL_START
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_log_edge(const int tol_factor) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::mt19937_64 gen;

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    std::uniform_real_distribution<float_type> dist(1.0F, 2.0F);

    volatile auto result_is_ok = true;

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_zero = log(::my_zero<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_zero_is_ok = (isinf(log_zero) && (log_zero < ::my_zero<decimal_type>()));

      BOOST_TEST(result_log_zero_is_ok);

      result_is_ok = (result_log_zero_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_zero_minus = log(-::my_zero<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_zero_minus_is_ok = (isinf(log_zero_minus) && (log_zero_minus < ::my_zero<decimal_type>()));

      BOOST_TEST(result_log_zero_minus_is_ok);

      result_is_ok = (result_log_zero_minus_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_one = log(::my_one<decimal_type>());

      const volatile auto result_log_one_is_ok = (log_one == ::my_zero<decimal_type>());

      BOOST_TEST(result_log_one_is_ok);

      result_is_ok = (result_log_one_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_one_minus = log(-::my_one<decimal_type>());

      const volatile auto result_log_one_minus_is_ok = isnan(log_one_minus);

      BOOST_TEST(result_log_one_minus_is_ok);

      result_is_ok = (result_log_one_minus_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_inf = log(::my_inf<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_inf_is_ok = isinf(log_inf);

      BOOST_TEST(result_log_inf_is_ok);

      result_is_ok = (result_log_inf_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_inf_minus = log(-::my_inf<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_inf_minus_is_ok = isnan(log_inf_minus);

      BOOST_TEST(result_log_inf_minus_is_ok);

      result_is_ok = (result_log_inf_minus_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_nan = log(std::numeric_limits<decimal_type>::quiet_NaN() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_nan_is_ok = isnan(log_nan);

      BOOST_TEST(result_log_nan_is_ok);

      result_is_ok = (result_log_nan_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      const auto x_flt = static_cast<float>(1.4L + static_cast<long double>(index) / 10.0L);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::log;

      const auto lg_flt = log(x_flt);
      const auto lg_dec = log(x_dec);

      const auto result_log_is_ok = is_close_fraction(lg_flt, static_cast<float>(lg_dec), std::numeric_limits<float>::epsilon() * static_cast<float>(tol_factor));

      BOOST_TEST(result_log_is_ok);

      result_is_ok = (result_log_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(9)); ++index)
    {
      const auto x_flt = static_cast<float>(0.1L + static_cast<long double>(index) / 10.0L);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::log;

      const auto lg_flt          = log(x_flt);
      const auto lg_dec          = log(x_dec);
      const auto lg_dec_as_float = static_cast<float>(lg_dec);

      const auto result_log_is_ok = is_close_fraction(lg_flt, lg_dec_as_float, std::numeric_limits<float>::epsilon() * static_cast<float>(tol_factor));

      BOOST_TEST(result_log_is_ok);

      result_is_ok = (result_log_is_ok && result_is_ok);
    }

    return result_is_ok;
  }
}

auto main() -> int
{
  auto result_is_ok = true;

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto test_log_is_ok                 = local::test_log                <decimal_type, float_type>(12);
    const auto test_log_between_1_and_2_is_ok = local::test_log_between_1_and_2<decimal_type, float_type>(24);
    const auto test_log_edge_is_ok            = local::test_log_edge           <decimal_type, float_type>(12);

    result_is_ok = (test_log_is_ok && test_log_between_1_and_2_is_ok && test_log_edge_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto test_log_is_ok                 = local::test_log                <decimal_type, float_type>(24);
    const auto test_log_between_1_and_2_is_ok = local::test_log_between_1_and_2<decimal_type, float_type>(256);
    const auto test_log_edge_is_ok            = local::test_log_edge           <decimal_type, float_type>(24);

    result_is_ok = (test_log_is_ok && test_log_between_1_and_2_is_ok && test_log_edge_is_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
