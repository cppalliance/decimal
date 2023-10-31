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
  auto test_tgamma(const int tol_factor, const long double range_lo, const long double range_hi) -> bool
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
      const auto x_flt = dis(gen);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::tgamma;

      const auto val_flt = tgamma(x_flt);
      const auto val_dec = tgamma(x_dec);

      const auto result_val_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), static_cast<float_type>(std::numeric_limits<decimal_type>::epsilon()) * tol_factor);

      result_is_ok = (result_val_is_ok && result_is_ok);

      if(!result_val_is_ok)
      {
        std::cout << "x_flt  : " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << x_flt   << std::endl;
        std::cout << "val_flt: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_flt << std::endl;
        std::cout << "val_dec: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_dec << std::endl;

        break;
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  auto test_tgamma_neg32(const int tol_factor) -> bool
  {
    // Table[N[Gamma[-23/100 - n], 32], {n, 1, 7, 1}]
    using ctrl_as_long_double_array_type = std::array<long double, static_cast<std::size_t>(UINT8_C(7))>;

    constexpr ctrl_as_long_double_array_type
      ctrl_values
      {
        static_cast<long double>(+4.2406941452013198921659716327521L),
        static_cast<long double>(-1.9016565673548519695811531985435L),
        static_cast<long double>(+0.58874816326775602773410315744382L),
        static_cast<long double>(-0.13918396294746005383784944620421L),
        static_cast<long double>(+0.026612612418252400351405247840194L),
        static_cast<long double>(-0.0042716873865573676326493174703360L),
        static_cast<long double>(+0.00059082813092079773618939384098700L)
      };

    auto result_is_ok = true;

    using decimal_type = boost::decimal::decimal32;
    using float_type = float;

    for(auto   n = static_cast<std::size_t>(UINT8_C(0));
               n < std::tuple_size<ctrl_as_long_double_array_type>::value;
             ++n)
    {
      const auto ld_arg = static_cast<long double>(-0.23L - static_cast<long double>(n + 1U));

      const auto x_dec = static_cast<boost::decimal::decimal32>(ld_arg);
      const auto x_flt = static_cast<float>(ld_arg);

      const auto val_flt = static_cast<float>(ctrl_values[n]);
      const auto val_dec = tgamma(x_dec);

      const auto result_val_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), static_cast<float_type>(std::numeric_limits<decimal_type>::epsilon()) * tol_factor);

      result_is_ok = (result_val_is_ok && result_is_ok);

      if(!result_val_is_ok)
      {
        std::cout << "x_flt  : " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << x_flt   << std::endl;
        std::cout << "val_flt: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_flt << std::endl;
        std::cout << "val_dec: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_dec << std::endl;

        break;
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  auto test_tgamma_edge() -> bool
  {
    auto result_is_ok = true;

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

} // namespace local

auto main() -> int
{
  auto result_is_ok = true;

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto result_tgamma_is_ok   = local::test_tgamma<decimal_type, float_type>(256, 0.1L, 21.7L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto result_tgamma_is_ok   = local::test_tgamma<decimal_type, float_type>(1024, 0.1L, 21.7L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    const auto result_neg32_is_ok = local::test_tgamma_neg32(512);

    BOOST_TEST(result_neg32_is_ok);

    result_is_ok = (result_neg32_is_ok && result_is_ok);
  }

  {
    const auto result_edge_is_ok = local::test_tgamma_edge();

    BOOST_TEST(result_edge_is_ok);

    result_is_ok = (result_edge_is_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}