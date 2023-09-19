// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#if defined(__clang__)
  #if defined __has_feature
  #if __has_feature(thread_sanitizer)
  #define BOOST_DECIMAL_REDUCE_TEST_DEPTH
  #endif
  #endif
#elif defined(__GNUC__)
  #if defined(__SANITIZE_THREAD__)
  #define BOOST_DECIMAL_REDUCE_TEST_DEPTH
  #endif
#elif defined(_MSC_VER)
  #if defined(_DEBUG)
  #define BOOST_DECIMAL_REDUCE_TEST_DEPTH
  #endif
#endif

#include <chrono>
#include <limits>
#include <random>

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

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
      const auto delta = fabs(1 - fabs(a / b));

      result_is_ok = (delta < tol);
    }

    return result_is_ok;
  }

  auto test_log10() -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    std::random_device rd;
    std::mt19937_64 gen(rd());

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    auto dis =
      std::uniform_real_distribution<float>
      {
        static_cast<float>(1.0E-17L),
        static_cast<float>(1.0E+17L)
      };

    auto result_is_ok = true;

    auto trials = static_cast<std::uint32_t>(UINT8_C(0));

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    constexpr auto count = static_cast<std::uint32_t>(UINT32_C(0x2000));
    #else
    constexpr auto count = static_cast<std::uint32_t>(UINT32_C(0x200));
    #endif

    for( ; trials < count; ++trials)
    {
      const auto x_flt = dis(gen);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::log;

      const auto lg_flt = log(x_flt);
      const auto lg_dec = log(x_dec);

      const auto result_log10_is_ok = is_close_fraction(lg_flt, static_cast<float>(lg_dec), std::numeric_limits<float>::epsilon() * 16);

      result_is_ok = (result_log10_is_ok && result_is_ok);

      if(!result_log10_is_ok)
      {
        std::cout << "x_flt : " <<                    x_flt  << std::endl;
        std::cout << "lg_flt: " << std::scientific << lg_flt << std::endl;
        std::cout << "lg_dec: " << std::scientific << lg_dec << std::endl;

        break;
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  auto test_log10_edge() -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    auto result_is_ok = true;

    {
      constexpr auto zero = decimal_type { 0 };
      constexpr auto one  = decimal_type { 1 };

      const auto log_one = log(one);

      const auto result_log_one_is_ok = (log_one == zero);

      BOOST_TEST(result_log_one_is_ok);

      result_is_ok = (result_log_one_is_ok && result_is_ok);
    }

    {
      const auto log_inf = log(std::numeric_limits<decimal_type>::infinity());

      const auto result_log_inf_is_ok = isinf(log_inf);

      BOOST_TEST(result_log_inf_is_ok);

      result_is_ok = (result_log_inf_is_ok && result_is_ok);
    }

    {
      const auto log_nan = log(std::numeric_limits<decimal_type>::quiet_NaN());

      const auto result_log_nan_is_ok = isnan(log_nan);

      BOOST_TEST(result_log_nan_is_ok);

      result_is_ok = (result_log_nan_is_ok && result_is_ok);
    }

    return result_is_ok;
  }
}

auto main() -> int
{
  auto result_is_ok = (local::test_log10() && local::test_log10_edge());

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
