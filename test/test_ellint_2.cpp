// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Propogates up from boost.math
#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

#include <boost/decimal.hpp>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wundef"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wsign-conversion"
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#  if __GNUC__ == 10
#  pragma GCC diagnostic ignored "-Wmisleading-indentation"
#  endif
#endif

#include <boost/math/special_functions/ellint_2.hpp>
#include <boost/math/special_functions/next.hpp>
#include <boost/core/lightweight_test.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>

template<typename DecimalType> auto my_zero() -> DecimalType&;
template<typename DecimalType> auto my_one () -> DecimalType&;
template<typename DecimalType> auto my_inf () -> DecimalType&;
template<typename DecimalType> auto my_nan () -> DecimalType&;

namespace local
{
  template<typename IntegralTimePointType,
           typename ClockType = std::chrono::high_resolution_clock>
  auto time_point() noexcept -> IntegralTimePointType;
}

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(64U);
#else
static constexpr auto N = static_cast<std::size_t>(8U);
#endif

static std::mt19937_64 rng(42);

using namespace boost::decimal;

template <typename T>
void test_comp_ellint()
{
  std::uniform_real_distribution<float> dist(-0.995F, 0.995F);

  constexpr auto local_N = N;

  for (std::size_t i {}; i < local_N; ++i)
  {
    const auto val {dist(rng)};
    const T dec_val {val};

    const auto float_res {boost::math::ellint_2(val)};
    const auto dec_res {static_cast<float>(comp_ellint_2(dec_val))};
    const auto distance {boost::math::float_distance(float_res, dec_res)};

    if (!BOOST_TEST(std::abs(distance) < 64))
    {
      // LCOV_EXCL_START
      std::cerr << "arg: " << dec_val
                << "\n Float: " << float_res
                << "\n  Dec: " << dec_res
                << "\n Dist: " << distance << std::endl;
      // LCOV_EXCL_STOP
    }
  }
}

template <typename T>
void test_ellint()
{
  std::uniform_real_distribution<float> dist_k  (-0.995F, 0.995F);
  std::uniform_real_distribution<float> dist_phi(-1.0F, 1.0F);

  rng.seed(local::time_point<typename std::mt19937_64::result_type>());

  constexpr auto local_N = N;

  for (std::size_t i {}; i < local_N; ++i)
  {
    const auto k_val {dist_k(rng)};
    const auto phi_val {dist_phi(rng)};

    const T k_dec_val {k_val};
    const T phi_dec_val {phi_val};

    const auto float_res {boost::math::ellint_2(k_val, phi_val)};
    const auto dec_res {static_cast<float>(ellint_2(k_dec_val, phi_dec_val))};
    const auto distance {boost::math::float_distance(float_res, dec_res)};

    if (!BOOST_TEST(std::abs(distance) < 384))
    {
      // LCOV_EXCL_START
      std::cerr << "Float: " << float_res
                << "\n  Dec: " << dec_res
                << "\n Dist: " << distance << std::endl;
      // LCOV_EXCL_STOP
    }
  }
}

namespace local
{
  template<typename IntegralTimePointType,
           typename ClockType>
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
  auto test_ellint_2_edge() -> bool
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

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      decimal_type arg_k_zero { ::my_zero<decimal_type>() };
      arg_k_zero *= static_cast<decimal_type>(dist(gen));

      decimal_type arg_phi { index + 1, -1 };

      const auto      ellint_2_k_zero =      ellint_2(arg_k_zero, arg_phi);
      const auto comp_ellint_2_k_zero = comp_ellint_2(arg_k_zero);

      const volatile auto result______ellint_2_k_zero_is_ok = (ellint_2_k_zero == arg_phi);
      const volatile auto result_comp_ellint_2_k_zero_is_ok =  local::is_close_fraction(comp_ellint_2_k_zero, numbers::pi_v<decimal_type> / 2, std::numeric_limits<decimal_type>::epsilon() * 16);

      BOOST_TEST(result______ellint_2_k_zero_is_ok);
      BOOST_TEST(result_comp_ellint_2_k_zero_is_ok);

      result_is_ok = (result______ellint_2_k_zero_is_ok && result_comp_ellint_2_k_zero_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      decimal_type arg_k_too_large { ::my_one<decimal_type>() + ::my_one<decimal_type>() };
      arg_k_too_large *= static_cast<decimal_type>(dist(gen));

      decimal_type arg_phi { index + 1, -1 };

      const auto      ellint_2_k_too_large =      ellint_2(arg_k_too_large, arg_phi);
      const auto comp_ellint_2_k_too_large = comp_ellint_2(arg_k_too_large);

      const volatile auto result______ellint_2_k_too_large_is_ok = isnan(     ellint_2_k_too_large);
      const volatile auto result_comp_ellint_2_k_too_large_is_ok = isnan(comp_ellint_2_k_too_large);

      BOOST_TEST(result______ellint_2_k_too_large_is_ok);
      BOOST_TEST(result_comp_ellint_2_k_too_large_is_ok);

      result_is_ok = (result______ellint_2_k_too_large_is_ok && result_comp_ellint_2_k_too_large_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      decimal_type arg_k_normal { index + 1, -1 };

      decimal_type arg_phi_zero { ::my_zero<decimal_type>() };
      arg_phi_zero *= static_cast<decimal_type>(dist(gen));

      const auto ellint_2_phi_zero = ellint_2(arg_k_normal, arg_phi_zero);

      const volatile auto result_ellint_2_phi_zero_is_ok = (fpclassify(ellint_2_phi_zero) == FP_ZERO);

      BOOST_TEST(result_ellint_2_phi_zero_is_ok);

      result_is_ok = (result_ellint_2_phi_zero_is_ok && result_is_ok);
    }

    for(auto index = static_cast<unsigned>(UINT8_C(0)); index < static_cast<unsigned>(UINT8_C(4)); ++index)
    {
      static_cast<void>(index);

      decimal_type arg_k_inf { ::my_inf<decimal_type>() };
      arg_k_inf *= static_cast<decimal_type>(dist(gen));

      decimal_type arg_phi_nan { ::my_nan<decimal_type>() };
      arg_phi_nan *= static_cast<decimal_type>(dist(gen));

      const auto ellint_2_nan = ellint_2(arg_k_inf, arg_phi_nan);

      const volatile auto result_ellint_2_nan_is_ok = isnan(ellint_2_nan);

      BOOST_TEST(result_ellint_2_nan_is_ok);

      result_is_ok = (result_ellint_2_nan_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  auto test_ellint_2_small_phi_64(const int tol_factor) -> bool
  {
    using decimal_type = boost::decimal::decimal64;

    using val_ctrl_array_type = std::array<double, 5U>;

    const val_ctrl_array_type ctrl_values =
    {{
      // Table[N[EllipticE[10^(-n), (1/3)^2], 17], {n, 5, 9, 1}]
      9.9999999999814815E-6,
      9.9999999999998148E-7,
      9.9999999999999981E-8,
      1.0000000000000000E-8,
      1.0000000000000000E-9
    }};

    std::array<decimal_type, std::tuple_size<val_ctrl_array_type>::value> ellint_2_values { };

    int nx { 5 };

    bool result_is_ok { true };

    const decimal_type my_tol { std::numeric_limits<decimal_type>::epsilon() * static_cast<decimal_type>(tol_factor) };

    for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < std::tuple_size<val_ctrl_array_type>::value; ++i)
    {
      // Table[N[EllipticF[10^(-n), (1/3)^2], 17], {n, 5, 9, 1}]

      const decimal_type phi_arg = { 1, -nx };

      ellint_2_values[i] = ellint_2(decimal_type { 1 } / 3, phi_arg);

      ++nx;

      const auto result_ellint_2_is_ok = is_close_fraction(ellint_2_values[i], decimal_type(ctrl_values[i]), my_tol);

      result_is_ok = (result_ellint_2_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  auto test_ellint_2_128_pos(const int tol_factor) -> bool
  {
    using decimal_type = boost::decimal::decimal128;

    using str_ctrl_array_type = std::array<const char*, 9U>;

    const str_ctrl_array_type ctrl_strings =
    {{
       // Table[N[EllipticE[2/3, (n/10 + n/100)^2], 36], {n, 1, 9, 1}]
       "0.666119665506243327585446597120365110",
       "0.664473943410175381838193351886861763",
       "0.661715101178958098391937979256508409",
       "0.657818296416741109456214296894271815",
       "0.652746855500265590541925917385113058",
       "0.646450050974753096902805338816260268",
       "0.638859667933305806172921631930756649",
       "0.629884681258945978370215239429076602",
       "0.619402771463409033787768967563501387",
    }};

    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> ellint_values { };
    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> ctrl_values   { };

    int nx { 1 };

    bool result_is_ok { true };

    const decimal_type my_tol { std::numeric_limits<decimal_type>::epsilon() * static_cast<decimal_type>(tol_factor) };

    for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < std::tuple_size<str_ctrl_array_type>::value; ++i)
    {
      const decimal_type
        k_arg
        {
            decimal_type { nx, -1 }
          + decimal_type { nx, -2 }
        };

      ++nx;

      ellint_values[i] = ellint_2(k_arg, decimal_type { 2 } / 3);

      static_cast<void>
      (
        from_chars(ctrl_strings[i], ctrl_strings[i] + std::strlen(ctrl_strings[i]), ctrl_values[i])
      );

      const auto result_log_is_ok = is_close_fraction(ellint_values[i], ctrl_values[i], my_tol);

      result_is_ok = (result_log_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  auto test_comp_ellint_2_128_pos(const int tol_factor) -> bool
  {
    using decimal_type = boost::decimal::decimal128;

    using str_ctrl_array_type = std::array<const char*, 19U>;

    const str_ctrl_array_type ctrl_strings =
    {{
       // Table[N[EllipticE[(n/10 + n/100)^2], 36], {n, -9, 9, 1}]
       "1.02847580902880400098388713851802174",
       "1.19543697705597932699632588347717453",
       "1.30226939374421038993312494156078132",
       "1.38238622197134778655786425086531964",
       "1.44424348825933199712071979564286290",
       "1.49176050930897376832354394465586437",
       "1.52711603392201517338661852946486612",
       "1.55161363259661260640786627054249108",
       "1.56603383286701375364685948074021920",
       "1.57079632679489661923132169163975144",
       "1.56603383286701375364685948074021920",
       "1.55161363259661260640786627054249108",
       "1.52711603392201517338661852946486612",
       "1.49176050930897376832354394465586437",
       "1.44424348825933199712071979564286290",
       "1.38238622197134778655786425086531964",
       "1.30226939374421038993312494156078132",
       "1.19543697705597932699632588347717453",
       "1.02847580902880400098388713851802174",
    }};

    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> ellint_values { };
    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> ctrl_values   { };

    int nx { -9 };

    bool result_is_ok { true };

    const decimal_type my_tol { std::numeric_limits<decimal_type>::epsilon() * static_cast<decimal_type>(tol_factor) };

    for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < std::tuple_size<str_ctrl_array_type>::value; ++i)
    {
      const decimal_type
        k_arg
        {
            decimal_type { nx, -1 }
          + decimal_type { nx, -2 }
        };

      ++nx;

      ellint_values[i] = comp_ellint_2(k_arg);

      static_cast<void>
      (
        from_chars(ctrl_strings[i], ctrl_strings[i] + std::strlen(ctrl_strings[i]), ctrl_values[i])
      );

      const auto result_log_is_ok = is_close_fraction(ellint_values[i], ctrl_values[i], my_tol);

      result_is_ok = (result_log_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

} // namespace local

int main()
{
  #if 0
  {
      using decimal_type = decimal128;

      const decimal_type k_arg   { decimal_type { 1 } / 3 };
      const decimal_type phi_arg { decimal_type { 1 } / 7 };

      const decimal_type ellint_2_val = ellint_2(k_arg, phi_arg);

      std::cout << "ellint_2_val: " << std::setprecision(std::numeric_limits<decimal_type>::digits10) << ellint_2_val << std::endl;
  }
  #endif

  test_comp_ellint<decimal32>();
  test_comp_ellint<decimal64>();

  test_ellint<decimal32>();
  test_ellint<decimal64>();

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto test_ellint_2_edge_is_ok = local::test_ellint_2_edge<decimal_type, float_type>();

    BOOST_TEST(test_ellint_2_edge_is_ok);
  }

  {
    const auto result_small_phi_64_is_ok = local::test_ellint_2_small_phi_64(256);

    BOOST_TEST(result_small_phi_64_is_ok);
  }

  {
    const auto result_pos128_is_ok = local::test_ellint_2_128_pos(0x400'000);

    BOOST_TEST(result_pos128_is_ok);
  }

  {
    const auto result_pos128_is_ok = local::test_comp_ellint_2_128_pos(0x10'000);

    BOOST_TEST(result_pos128_is_ok);
  }

  return boost::report_errors();
}

template<typename DecimalType> auto my_zero() -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_zero { 0 }; return val_zero; }
template<typename DecimalType> auto my_one () -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_one  { 1 }; return val_one; }
template<typename DecimalType> auto my_inf () -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_inf  { std::numeric_limits<decimal_type>::infinity() }; return val_inf; }
template<typename DecimalType> auto my_nan () -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_nan  { std::numeric_limits<decimal_type>::quiet_NaN() }; return val_nan; }
