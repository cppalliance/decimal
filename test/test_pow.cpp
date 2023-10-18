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
      const auto delta = fabs(1 - fabs(a / b));

      result_is_ok = (delta < tol);
    }

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_pow(const int tol_factor, const bool neg_a) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::random_device rd;
    std::mt19937_64 gen(rd());

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    auto dis_x =
      std::uniform_real_distribution<float_type>
      {
        static_cast<float_type>(1.0E-1L),
        static_cast<float_type>(1.0E+1L)
      };

    auto dis_a =
      std::uniform_real_distribution<float_type>
      {
        static_cast<float_type>(0.0123L),
        static_cast<float_type>(12.3L)
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
      float_type a_flt_begin { };
      float_type x_flt_begin { };

      do { a_flt_begin = dis_a(gen); } while(a_flt_begin == 0);
      do { x_flt_begin = dis_x(gen); } while(x_flt_begin == 0);

      const auto a_flt = ((!neg_a) ? a_flt_begin : -a_flt_begin);
      const auto a_dec = static_cast<decimal_type>(a_flt);

      const auto x_flt = x_flt_begin;
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::pow;

      const auto val_flt = pow(x_flt, a_flt);
      const auto val_dec = pow(x_dec, a_dec);

      const auto result_val_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), std::numeric_limits<float_type>::epsilon() * tol_factor);

      result_is_ok = (result_val_is_ok && result_is_ok);

      if(!result_val_is_ok)
      {
        std::cout << "x_flt  : " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << x_flt   << std::endl;
        std::cout << "a_flt  : " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << a_flt   << std::endl;
        std::cout << "val_flt: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_flt << std::endl;
        std::cout << "val_dec: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_dec << std::endl;

        break;
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_pow_edge(const int tol_factor) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::mt19937_64 gen;

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    std::uniform_real_distribution<float_type>
      dist
      (
        static_cast<float_type>(1.01L),
        static_cast<float_type>(1.04L)
      );

    volatile auto result_is_ok = true;

    for(auto index = static_cast<int>(INT8_C(-11)); index <= static_cast<int>(UINT8_C(-3)); index += static_cast<int>(INT8_C(2)))
    {
      const auto val_zero_pos = pow(::my_zero<decimal_type>(), decimal_type(index));

      const volatile auto result_val_zero_pos_is_ok = (isinf(val_zero_pos) && (!signbit(val_zero_pos)));

      BOOST_TEST(result_val_zero_pos_is_ok);

      result_is_ok = (result_val_zero_pos_is_ok && result_is_ok);
    }

    for(auto index = static_cast<int>(INT8_C(-11)); index <= static_cast<int>(UINT8_C(-3)); index += static_cast<int>(INT8_C(2)))
    {
      const auto val_zero_neg = pow(-::my_zero<decimal_type>(), decimal_type(index));

      const volatile auto result_val_zero_neg_is_ok = (isinf(val_zero_neg) && (signbit(val_zero_neg)));

      BOOST_TEST(result_val_zero_neg_is_ok);

      result_is_ok = (result_val_zero_neg_is_ok && result_is_ok);
    }

    for(auto index = static_cast<int>(INT8_C(-10)); index <= static_cast<int>(UINT8_C(-2)); index += static_cast<int>(INT8_C(2)))
    {
      const auto val_zero_neg = pow(-::my_zero<decimal_type>(), decimal_type(index));

      const volatile auto result_val_zero_neg_is_ok = (isinf(val_zero_neg) && (!signbit(val_zero_neg)));

      BOOST_TEST(result_val_zero_neg_is_ok);

      result_is_ok = (result_val_zero_neg_is_ok && result_is_ok);
    }

    #if 0
    for(auto index = static_cast<int>(INT8_C(-10)); index <= static_cast<int>(UINT8_C(-2)); index += static_cast<int>(INT8_C(2)))
    {
      const auto val_zero_neg = pow(::my_zero<decimal_type>(), decimal_type(index));

      const volatile auto result_val_zero_neg_is_ok = (isinf(val_zero_neg) && (!signbit(val_zero_neg)));

      BOOST_TEST(result_val_zero_neg_is_ok);

      result_is_ok = (result_val_zero_neg_is_ok && result_is_ok);
    }
    #endif

    for(auto index = static_cast<int>(INT8_C(-10)); index <= static_cast<int>(UINT8_C(-2)); index += static_cast<int>(INT8_C(2)))
    {
      const auto val_zero_neg = pow(-::my_zero<decimal_type>(), -static_cast<decimal_type>(dist(gen)));

      const volatile auto result_val_zero_neg_is_ok = (isinf(val_zero_neg) && (!signbit(val_zero_neg)));

      BOOST_TEST(result_val_zero_neg_is_ok);

      result_is_ok = (result_val_zero_neg_is_ok && result_is_ok);
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

    const auto test_pow_edge_is_ok = local::test_pow_edge<decimal_type, float_type>(512);
    const auto test_pow_pos_is_ok  = local::test_pow     <decimal_type, float_type>(512, false);
    const auto test_pow_neg_is_ok  = local::test_pow     <decimal_type, float_type>(512, true);

    result_is_ok = (test_pow_pos_is_ok && test_pow_neg_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto test_pow_pos_is_ok = local::test_pow<decimal_type, float_type>(4096, false);
    const auto test_pow_is_neg_ok = local::test_pow<decimal_type, float_type>(4096, true);

    result_is_ok = (test_pow_pos_is_ok && test_pow_is_neg_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
