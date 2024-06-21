// Copyright 2023 - 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
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

template<typename DecimalType> auto my_zero() -> DecimalType&;
template<typename DecimalType> auto my_one () -> DecimalType&;
template<typename DecimalType> auto my_nan () -> DecimalType&;

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

    bool result_is_ok { true };

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

      const auto result_val_is_ok = is_close_fraction(val_flt, static_cast<float_type>(val_dec), static_cast<float_type>(std::numeric_limits<decimal_type>::epsilon()) * static_cast<float_type>(tol_factor));

      result_is_ok = (result_val_is_ok && result_is_ok);

      if(!result_val_is_ok)
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

  auto test_tgamma_neg32(const int tol_factor) -> bool
  {
    // Table[N[Gamma[-23/100 - n], 32], {n, 1, 7, 1}]
    using ctrl_as_long_double_array_type = std::array<long double, static_cast<std::size_t>(UINT8_C(7))>;

    constexpr auto ctrl_values =
      ctrl_as_long_double_array_type
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

  auto test_tgamma_small_ui32() -> bool
  {
    // Table[Gamma[n], {n, 1, 9, 1}]
    using data_as_uint_pair_array_type = std::array<std::pair<unsigned, unsigned>, static_cast<std::size_t>(UINT8_C(9))>;

    constexpr auto values =
      data_as_uint_pair_array_type
      {
        std::pair<unsigned, unsigned> { 1U,     1 },
        std::pair<unsigned, unsigned> { 2U,     1 },
        std::pair<unsigned, unsigned> { 3U,     2 },
        std::pair<unsigned, unsigned> { 4U,     6 },
        std::pair<unsigned, unsigned> { 5U,    24 },
        std::pair<unsigned, unsigned> { 6U,   120 },
        std::pair<unsigned, unsigned> { 7U,   720 },
        std::pair<unsigned, unsigned> { 8U,  5040 },
        std::pair<unsigned, unsigned> { 9U, 40320 }
      };

    auto result_is_ok = true;

    for(auto& pr : values)
    {
      const auto tg = tgamma( boost::decimal::decimal32 { pr.first, 0 } );

      const auto result_tg_is_ok = ((static_cast<unsigned>(tg) == pr.second) && (static_cast<unsigned>(tg) == tg));

      result_is_ok = (result_tg_is_ok && result_is_ok);
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  namespace detail
  {
    auto local_factorial(const int u_arg) -> int
    {
      return ((u_arg > 1) ? u_arg * local_factorial(u_arg - 1) : 1);
    }
  } // namespace detail

  template<typename DecimalType, typename FloatType>
  auto test_tgamma_edge() -> bool
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

    auto result_is_ok = true;

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(8)); ++i)
    {
      static_cast<void>(i);

      const decimal_type arg_nan = ::my_nan<decimal_type>() * static_cast<decimal_type>(dist(gen));

      const auto val_nan = tgamma(arg_nan);

      const auto result_val_nan_is_ok = (isnan(arg_nan) && isnan(val_nan));

      BOOST_TEST(result_val_nan_is_ok);

      result_is_ok = (result_val_nan_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_inf_pos = tgamma(std::numeric_limits<decimal_type>::infinity() * static_cast<decimal_type>(dist(gen)));

      const auto result_val_inf_pos_is_ok = isinf(val_inf_pos);

      BOOST_TEST(result_val_inf_pos_is_ok);

      result_is_ok = (result_val_inf_pos_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_inf_neg = tgamma(-std::numeric_limits<decimal_type>::infinity() * static_cast<decimal_type>(dist(gen)));

      const auto result_val_inf_neg_is_ok = isnan(val_inf_neg);

      BOOST_TEST(result_val_inf_neg_is_ok);

      result_is_ok = (result_val_inf_neg_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_zero_pos = tgamma(::my_zero<decimal_type>());

      const auto result_val_zero_pos_is_ok = (isinf(val_zero_pos) && (!signbit(val_zero_pos)));

      BOOST_TEST(result_val_zero_pos_is_ok);

      result_is_ok = (result_val_zero_pos_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(4)); ++i)
    {
      static_cast<void>(i);

      const auto val_zero_neg = tgamma(-(::my_zero<decimal_type>() * static_cast<decimal_type>(dist(gen))));

      const auto result_val_zero_neg_is_ok = (isinf(val_zero_neg) && signbit(val_zero_neg));

      BOOST_TEST(result_val_zero_neg_is_ok);

      result_is_ok = (result_val_zero_neg_is_ok && result_is_ok);
    }

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(6)); ++i)
    {
      decimal_type dnx { ::my_zero<decimal_type>() * static_cast<decimal_type>(dist(gen)) };

      dnx -= static_cast<int>(i + 1);

      const auto val_neg_int = tgamma(dnx);

      const auto result_val_neg_int_is_ok = isnan(val_neg_int);

      BOOST_TEST(result_val_neg_int_is_ok);

      result_is_ok = (result_val_neg_int_is_ok && result_is_ok);
    }

    {
      std::uniform_int_distribution<int> n_dist(-8, 8);

      for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(64)); ++i)
      {
        static_cast<void>(i);

        int n_arg { };

        do
        {
          n_arg = static_cast<int>(dist(gen) * n_dist(gen));
        }
        while(n_arg == 0);

        const auto val_neg_or_pos_int = tgamma(decimal_type { n_arg });

        const bool result_val_neg_or_pos_int_is_ok
        {
          (n_arg < 0)
            ? isnan(val_neg_or_pos_int)
            : (val_neg_or_pos_int == decimal_type { detail::local_factorial(n_arg - 1) })
        };

        BOOST_TEST(result_val_neg_or_pos_int_is_ok);
      }
    }

    return result_is_ok;
  }

  auto test_tgamma_64(const int tol_factor) -> bool
  {
    using decimal_type = boost::decimal::decimal64;

    using val_ctrl_array_type = std::array<double, 9U>;

    const val_ctrl_array_type ctrl_values =
    {{
      // Table[N[Gamma[(100 n + 10 n + 1)/100], 17], {n, 1, 9, 1}]
      0.94739550403930194,
      1.1078475565340642,
      2.7113982392439032,
      10.275404092015205,
      53.193428252500821,
      350.99860982420059,
      2825.0945368041871,
      26903.671946749768,
      296439.08210247219
    }};

    std::array<decimal_type, std::tuple_size<val_ctrl_array_type>::value> tgamma_values { };

    int nx { 1 };

    bool result_is_ok { true };

    const decimal_type my_tol { std::numeric_limits<decimal_type>::epsilon() * static_cast<decimal_type>(tol_factor) };

    for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < std::tuple_size<val_ctrl_array_type>::value; ++i)
    {
      // Table[N[Gamma[(100 n + 10 n + 1)/100], 17], {n, 1, 9, 1}]

      const decimal_type x_arg =
        decimal_type
        {
            decimal_type { 1, 2 } * nx
          + decimal_type { 1, 1 } * nx
          + 1
        }
        / decimal_type { 1, 2 };


      tgamma_values[i] = tgamma(x_arg);

      ++nx;

      const auto result_tgamma_is_ok = is_close_fraction(tgamma_values[i], decimal_type(ctrl_values[i]), my_tol);

      result_is_ok = (result_tgamma_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  auto test_tgamma_128_lo(const int tol_factor) -> bool
  {
    using decimal_type = boost::decimal::decimal128;

    using str_ctrl_array_type = std::array<const char*, 9U>;

    const str_ctrl_array_type ctrl_strings =
    {{
       // Table[N[Gamma[n/10 + n/100], 36], {n, 1, 9, 1}]
       "8.61268640035729038303843315710385452",
       "4.15048157959277857782635113344664974",
       "2.70720622261519104902052213245593595",
       "2.01319332601838966777117106234059403",
       "1.61612426873357513405845849344452552",
       "1.36616419875147485749818904751902063",
       "1.19969237367745339749375337490556205",
       "1.08530778746771950916024031037404015",
       "1.00587197964410779193412655924290279"
    }};

    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> tg_values   { };
    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> ctrl_values { };

    int nx { 1 };

    bool result_is_ok { true };

    const decimal_type my_tol { std::numeric_limits<decimal_type>::epsilon() * static_cast<decimal_type>(tol_factor) };

    for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < std::tuple_size<str_ctrl_array_type>::value; ++i)
    {
      const decimal_type x_arg =
        decimal_type
        {
            decimal_type { nx, -1 }
          + decimal_type { nx, -2 }
        };

        ++nx;

        tg_values[i] = tgamma(x_arg);

        static_cast<void>
        (
          from_chars(ctrl_strings[i], ctrl_strings[i] + std::strlen(ctrl_strings[i]), ctrl_values[i])
        );

      const auto result_tgamma_is_ok = is_close_fraction(tg_values[i], ctrl_values[i], my_tol);

      result_is_ok = (result_tgamma_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  auto test_tgamma_128_hi(const int tol_factor) -> bool
  {
    using decimal_type = boost::decimal::decimal128;

    using str_ctrl_array_type = std::array<const char*, 23U>;

    const str_ctrl_array_type ctrl_strings =
    {{
       // Table[N[Gamma[n + n/10 + n/100 + n/1000], 36], {n, 1, 221, 10}]
       "0.947008281162266001895790481785841941",
       "6.86303089001025022525468906807854872E7",
       "3.15793281780505944512262743601561476E21",
       "4.09725124531962875389920397572482227E37",
       "2.15936518595728901631037627967671095E55",
       "1.81286283067020212427848823649632939E74",
       "1.39061339788491577387400422516492967E94",
       "6.73844979762045895677263594960237945E114",
       "1.58535690838444528565837326081067457E136",
       "1.48677291673153228216478665408262025E158",
       "4.76763037027821868276349648015607359E180",
       "4.62395515046183569847627307320617350E203",
       "1.22680267570425015175034111397510637E227",
       "8.18925182002285090986591692926519438E250",
       "1.28134405415265103961333749220602490E275",
       "4.42253704896092684478952587741331734E299",
       "3.19451354412535995695298989136255493E324",
       "4.61171076932972412633999770033353340E349",
       "1.27758574231803927960543278875893523E375",
       "6.55079490827236721494047351435261992E400",
       "6.01906299656231025481256209731706244E426",
       "9.62614024174757375775890458257288037E452",
       "2.60989891797040728048724526392884050E479",
    }};

    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> tg_values   { };
    std::array<decimal_type, std::tuple_size<str_ctrl_array_type>::value> ctrl_values { };

    int nx { 1 };

    bool result_is_ok { true };

    const decimal_type my_tol { std::numeric_limits<decimal_type>::epsilon() * static_cast<decimal_type>(tol_factor) };

    for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < std::tuple_size<str_ctrl_array_type>::value; ++i)
    {
      const decimal_type x_arg =
        decimal_type
        {
            decimal_type { nx }
          + decimal_type { nx, -1 }
          + decimal_type { nx, -2 }
          + decimal_type { nx, -3 }
        };

        nx += 10;

        tg_values[i] = tgamma(x_arg);

        static_cast<void>
        (
          from_chars(ctrl_strings[i], ctrl_strings[i] + std::strlen(ctrl_strings[i]), ctrl_values[i])
        );

      const auto result_tgamma_is_ok = is_close_fraction(tg_values[i], ctrl_values[i], my_tol);

      result_is_ok = (result_tgamma_is_ok && result_is_ok);
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

    const auto result_tgamma_is_ok   = local::test_tgamma<decimal_type, float_type>(768, 0.01L, 0.9L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto result_tgamma_is_ok   = local::test_tgamma<decimal_type, float_type>(768, 2.1L, 23.4L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal32_fast;
    using float_type   = float;

    const auto result_tgamma_is_ok   = local::test_tgamma<decimal_type, float_type>(768, 2.1L, 23.4L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto result_tgamma_is_ok   = local::test_tgamma<decimal_type, float_type>(4096, 0.001L, 0.9L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto result_tgamma_is_ok   = local::test_tgamma<decimal_type, float_type>(4096, 2.1L, 78.9L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    const auto result_neg32_is_ok = local::test_tgamma_neg32(768);

    BOOST_TEST(result_neg32_is_ok);

    result_is_ok = (result_neg32_is_ok && result_is_ok);
  }

  {
    const auto result_ui32_is_ok = local::test_tgamma_small_ui32();

    BOOST_TEST(result_ui32_is_ok);

    result_is_ok = (result_ui32_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto result_edge_is_ok = local::test_tgamma_edge<decimal_type, float_type>();

    BOOST_TEST(result_edge_is_ok);

    result_is_ok = (result_edge_is_ok && result_is_ok);
  }

  {
    const auto result_tgamma64_is_ok   = local::test_tgamma_64(256);

    BOOST_TEST(result_tgamma64_is_ok);

    result_is_ok = (result_tgamma64_is_ok && result_is_ok);
  }

  {
    const auto result_tgamma128_lo_is_ok   = local::test_tgamma_128_lo(512);
    const auto result_tgamma128_hi_is_ok   = local::test_tgamma_128_hi(0x20'000);

    BOOST_TEST(result_tgamma128_lo_is_ok);
    BOOST_TEST(result_tgamma128_hi_is_ok);

    result_is_ok = (result_tgamma128_lo_is_ok && result_tgamma128_hi_is_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}

template<typename DecimalType> auto my_zero() -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_zero { 0 }; return val_zero; }
template<typename DecimalType> auto my_one () -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_one  { 1 }; return val_one; }
template<typename DecimalType> auto my_nan () -> DecimalType& { using decimal_type = DecimalType; static decimal_type val_nan  { std::numeric_limits<decimal_type>::quiet_NaN() }; return val_nan; }
