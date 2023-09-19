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

  auto test_log() -> bool
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

      const auto result_log_is_ok = is_close_fraction(lg_flt, static_cast<float>(lg_dec), std::numeric_limits<float>::epsilon() * 16);

      result_is_ok = (result_log_is_ok && result_is_ok);

      if(!result_log_is_ok)
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

  namespace constants {

  auto my_zero() -> const boost::decimal::decimal32& { static const boost::decimal::decimal32& my_zero { 0, 0 }; return my_zero; }
  auto my_one () -> const boost::decimal::decimal32& { static const boost::decimal::decimal32& my_one  { 1, 0 }; return my_one; }
  auto my_inf () -> const boost::decimal::decimal32& { static const boost::decimal::decimal32& my_inf  { std::numeric_limits<boost::decimal::decimal32>::infinity() };  return my_inf; }
  auto my_nan () -> const boost::decimal::decimal32& { static const boost::decimal::decimal32& my_nan  { std::numeric_limits<boost::decimal::decimal32>::quiet_NaN() }; return my_nan; }

  } // namespace constants

  #if (defined(__GNUC__) && !defined(__clang__))
  #pragma GCC push_options
  #pragma GCC optimize ("O0")
  #endif

  auto test_log_edge() -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    volatile auto result_is_ok = true;

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(5)); ++index)
    {
      static_cast<void>(index);

      const auto log_one = log(constants::my_one());

      const volatile auto result_log_one_is_ok = (log_one == constants::my_zero());

      BOOST_TEST(result_log_one_is_ok);

      result_is_ok = (result_log_one_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(5)); ++index)
    {
      static_cast<void>(index);

      const auto log_inf = log(constants::my_inf());

      const volatile auto result_log_inf_is_ok = isinf(log_inf);

      assert(isinf(log_inf));

      BOOST_TEST(result_log_inf_is_ok);

      result_is_ok = (result_log_inf_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(5)); ++index)
    {
      static_cast<void>(index);

      const auto log_nan = log(std::numeric_limits<decimal_type>::quiet_NaN());

      const volatile auto result_log_nan_is_ok = isnan(constants::my_nan());

      assert(isnan(log_nan));

      BOOST_TEST(result_log_nan_is_ok);

      result_is_ok = (result_log_nan_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(5)); ++index)
    {
      const auto x_flt = static_cast<float>(1.4L + static_cast<long double>(index) / 10.0L);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::log;

      const auto lg_flt = log(x_flt);
      const auto lg_dec = log(x_dec);

      const auto result_log_is_ok = is_close_fraction(lg_flt, static_cast<float>(lg_dec), std::numeric_limits<float>::epsilon() * 16);

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

      const auto result_log_is_ok = is_close_fraction(lg_flt, lg_dec_as_float, std::numeric_limits<float>::epsilon() * 24);

      BOOST_TEST(result_log_is_ok);

      result_is_ok = (result_log_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  #if (defined(__GNUC__) && !defined(__clang__))
  #pragma GCC pop_options
  #endif
}

auto main() -> int
{
  auto result_is_ok = (local::test_log() && local::test_log_edge());

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
