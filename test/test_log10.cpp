// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>

#include <boost/decimal.hpp>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/core/lightweight_test.hpp>

template<typename DecimalType> auto my_zero() -> DecimalType&;
template<typename DecimalType> auto my_one () -> DecimalType&;
template<typename DecimalType> auto my_inf () -> DecimalType&;

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

    NumericType delta { };

    if(b == static_cast<NumericType>(0))
    {
      delta = fabs(a - b); // LCOV_EXCL_LINE

      result_is_ok = (delta < tol); // LCOV_EXCL_LINE
    }
    else
    {
      delta = fabs(1 - (a / b));

      result_is_ok = (delta < tol);
    }

    // LCOV_EXCL_START
    if (!result_is_ok)
    {
      std::cerr << std::setprecision(std::numeric_limits<NumericType>::digits10) << "a: " << a
                << "\nb: " << b
                << "\ndelta: " << delta
                << "\ntol: " << tol << std::endl;
    }
    // LCOV_EXCL_STOP

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_log10(const int tol_factor) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::random_device rd;
    std::mt19937_64 gen(rd());

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    auto dis_r =
      std::uniform_real_distribution<float_type>
      {
        static_cast<float_type>(1.4L),
        static_cast<float_type>(8.9L)
      };

    bool result_is_ok { true };

    auto trials = static_cast<std::uint32_t>(UINT8_C(0));

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    constexpr auto count = (sizeof(decimal_type) == static_cast<std::size_t>(UINT8_C(4))) ? static_cast<std::uint32_t>(UINT32_C(0x200)) : static_cast<std::uint32_t>(UINT32_C(0x40));
    #else
    constexpr auto count = (sizeof(decimal_type) == static_cast<std::size_t>(UINT8_C(4))) ? static_cast<std::uint32_t>(UINT32_C(0x40)) : static_cast<std::uint32_t>(UINT32_C(0x4));
    #endif

    for( ; trials < count; ++trials)
    {
      auto x_flt = dis_r(gen);

      auto dis_n =
        std::uniform_int_distribution<int>
        {
          static_cast<int>(INT8_C(-17)),
          static_cast<int>(INT8_C(17))
        };

      std::string str_e { "1.0E" + std::to_string(dis_n(gen)) };

      char* p_end;

      x_flt *= static_cast<float>(strtold(str_e.c_str(), &p_end));

      static_cast<void>(p_end);

      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::log10;

      const auto val_flt = log10(x_flt);
      const auto val_dec = log10(x_dec);

      const auto result_log_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), std::numeric_limits<float_type>::epsilon() * static_cast<float_type>(tol_factor));

      result_is_ok = (result_log_is_ok && result_is_ok);

      if(!result_log_is_ok)
      {
        // LCOV_EXCL_START
        std::cerr << "x_flt  : " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << x_flt   << std::endl;
        std::cerr << "val_flt: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_flt << std::endl;
        std::cerr << "val_dec: " << std::scientific << std::setprecision(std::numeric_limits<float_type>::digits10) << val_dec << std::endl;

        break;
        // LCOV_EXCL_STOP
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_log10_pow10() -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    bool result_is_ok { true };

    for(int i = static_cast<int>(INT8_C(-23)); i <= static_cast<int>(INT8_C(23)); ++i)
    {
      const decimal_type x_arg { 1, i };

      const decimal_type val_dec  = log10(x_arg);
      const float_type   val_ctrl =  static_cast<float_type>(i);

      const float_type val_to_check = static_cast<float_type>(val_dec);

      const auto result_log10_pow10_is_ok = (val_to_check == val_ctrl);

      result_is_ok = (result_log10_pow10_is_ok && result_is_ok);
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  template<typename DecimalType, typename FloatType>
  auto test_log10_edge() -> bool
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

      const auto log_zero = log10(::my_zero<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_zero_is_ok = (isinf(log_zero) && (log_zero < ::my_zero<decimal_type>()));

      BOOST_TEST(result_log_zero_is_ok);

      result_is_ok = (result_log_zero_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_zero_minus = log10(-::my_zero<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_zero_minus_is_ok = (isinf(log_zero_minus) && (log_zero_minus < ::my_zero<decimal_type>()));

      BOOST_TEST(result_log_zero_minus_is_ok);

      result_is_ok = (result_log_zero_minus_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_one = log10(::my_one<decimal_type>());

      const volatile auto result_log_one_is_ok = (log_one == ::my_zero<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      BOOST_TEST(result_log_one_is_ok);

      result_is_ok = (result_log_one_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_one_minus = log10(-::my_one<decimal_type>());

      const volatile auto result_log_one_minus_is_ok = isnan(log_one_minus);

      BOOST_TEST(result_log_one_minus_is_ok);

      result_is_ok = (result_log_one_minus_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_inf = log10(::my_inf<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_inf_is_ok = isinf(log_inf);

      BOOST_TEST(result_log_inf_is_ok);

      result_is_ok = (result_log_inf_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_inf_minus = log10(-::my_inf<decimal_type>() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_inf_minus_is_ok = isnan(log_inf_minus);

      BOOST_TEST(result_log_inf_minus_is_ok);

      result_is_ok = (result_log_inf_minus_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      const auto log_nan = log10(std::numeric_limits<decimal_type>::quiet_NaN() * static_cast<decimal_type>(dist(gen)));

      const volatile auto result_log_nan_is_ok = isnan(log_nan);

      BOOST_TEST(result_log_nan_is_ok);

      result_is_ok = (result_log_nan_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  auto test_log10_128(const int tol_factor) -> bool
  {
    using decimal_type = boost::decimal::decimal128;

    using str_ctrl_array_type = std::array<const char*, 28U>;

    const str_ctrl_array_type ctrl_strings =
    {{
       // Table[N[Log[10, 456 10^n], 36], {n, -3, 24, 1}]
       "-0.341035157335565015527421936814476293",
       "0.658964842664434984472578063185523707",
       "1.65896484266443498447257806318552371",
       "2.65896484266443498447257806318552371",
       "3.65896484266443498447257806318552371",
       "4.65896484266443498447257806318552371",
       "5.65896484266443498447257806318552371",
       "6.65896484266443498447257806318552371",
       "7.65896484266443498447257806318552371",
       "8.65896484266443498447257806318552371",
       "9.65896484266443498447257806318552371",
       "10.6589648426644349844725780631855237",
       "11.6589648426644349844725780631855237",
       "12.6589648426644349844725780631855237",
       "13.6589648426644349844725780631855237",
       "14.6589648426644349844725780631855237",
       "15.6589648426644349844725780631855237",
       "16.6589648426644349844725780631855237",
       "17.6589648426644349844725780631855237",
       "18.6589648426644349844725780631855237",
       "19.6589648426644349844725780631855237",
       "20.6589648426644349844725780631855237",
       "21.6589648426644349844725780631855237",
       "22.6589648426644349844725780631855237",
       "23.6589648426644349844725780631855237",
       "24.6589648426644349844725780631855237",
       "25.6589648426644349844725780631855237",
       "26.6589648426644349844725780631855237",
    }};

    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> log_values { };
    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> ctrl_values { };

    int nx { -3 };

    bool result_is_ok { true };

    const decimal_type my_tol { std::numeric_limits<decimal_type>::epsilon() * static_cast<decimal_type>(tol_factor) };

    for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < std::tuple_size<str_ctrl_array_type>::value; ++i)
    {
      const decimal_type x_arg { 456, nx };

      ++nx;

      log_values[i] = log10(x_arg);

      static_cast<void>
      (
        from_chars(ctrl_strings[i], ctrl_strings[i] + std::strlen(ctrl_strings[i]), ctrl_values[i])
      );

      const auto result_log_is_ok = is_close_fraction(log_values[i], ctrl_values[i], my_tol);

      result_is_ok = (result_log_is_ok && result_is_ok);
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

    const auto test_log10_is_ok = local::test_log10<decimal_type, float_type>(64);

    BOOST_TEST(test_log10_is_ok);

    result_is_ok = (test_log10_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto test_log10_is_ok = local::test_log10<decimal_type, float_type>(256);

    BOOST_TEST(test_log10_is_ok);

    result_is_ok = (test_log10_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto test_log10_pow10_is_ok = local::test_log10_pow10<decimal_type, float_type>();

    BOOST_TEST(test_log10_pow10_is_ok);

    result_is_ok = (test_log10_pow10_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto test_log10_edge_is_ok = local::test_log10_edge<decimal_type, float_type>();

    BOOST_TEST(test_log10_edge_is_ok);

    result_is_ok = (test_log10_edge_is_ok && result_is_ok);
  }

  {
    const auto result_pos128_is_ok = local::test_log10_128(1'400'000);

    BOOST_TEST(result_pos128_is_ok);

    result_is_ok = (result_pos128_is_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}

template<typename DecimalType>
auto my_zero() -> DecimalType&
{
  using decimal_type = DecimalType;

  static decimal_type val_zero { 0, 0 };

  return val_zero;
}

template<typename DecimalType>
auto my_one() -> DecimalType&
{
  using decimal_type = DecimalType;

  static decimal_type val_one { 1, 0 };

  return val_one;
}

template<typename DecimalType>
auto my_inf() -> DecimalType&
{
  using decimal_type = DecimalType;

  static decimal_type val_inf { std::numeric_limits<decimal_type>::infinity() };

  return val_inf;
}
