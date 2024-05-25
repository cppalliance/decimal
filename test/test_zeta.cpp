// Copyright 2024 Matt Borland
// Copyright 2024 Christoper Kormanyos
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
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/decimal.hpp>

#include <boost/core/lightweight_test.hpp>
#include <boost/math/special_functions/zeta.hpp>

#include <iomanip>
#include <iostream>
#include <random>

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
  auto test_riemann_zeta(const int tol_factor) -> bool
  {
    using decimal_type = DecimalType;
    using float_type   = FloatType;

    std::random_device rd;
    std::mt19937_64 gen(rd());

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    auto dis_r =
      std::uniform_real_distribution<float_type>
      {
        static_cast<float_type>(1.001L),
        static_cast<float_type>(12.3)
      };

    bool result_is_ok { true };

    auto trials = static_cast<std::uint32_t>(UINT8_C(0));

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    constexpr auto count = (sizeof(decimal_type) == static_cast<std::size_t>(UINT8_C(4))) ? static_cast<std::uint32_t>(UINT32_C(0x80)) : static_cast<std::uint32_t>(UINT32_C(0x20));
    #else
    constexpr auto count = (sizeof(decimal_type) == static_cast<std::size_t>(UINT8_C(4))) ? static_cast<std::uint32_t>(UINT32_C(0x10)) : static_cast<std::uint32_t>(UINT32_C(0x4));
    #endif

    for( ; trials < count; ++trials)
    {
      const auto x_flt = dis_r(gen);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      const auto val_flt = boost::math::zeta(x_flt);
      const auto val_dec = riemann_zeta(x_dec);

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
auto test_riemann_zeta_edge() -> void
{
  using decimal_type = DecimalType;
  using float_type   = FloatType;

  {
    std::mt19937_64 gen;

    std::uniform_real_distribution<float_type> dist(static_cast<float_type>(1.1L), static_cast<float_type>(101.1L));

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(10)); ++i)
    {
      static_cast<void>(i);

      const decimal_type inf  { std::numeric_limits<decimal_type>::infinity() * static_cast<int>(dist(gen)) };
      const decimal_type nan  { std::numeric_limits<decimal_type>::quiet_NaN() * static_cast<int>(dist(gen)) };
      const decimal_type zero { decimal_type { 0 } * static_cast<int>(dist(gen)) };

      BOOST_TEST_EQ(riemann_zeta(inf), decimal_type { 1 } );
      BOOST_TEST   (isinf(riemann_zeta(-inf)) && signbit(riemann_zeta(-inf)));
      BOOST_TEST   (isnan(riemann_zeta(nan)));
      BOOST_TEST   (isnan(riemann_zeta(-nan)));
      BOOST_TEST   (isnan(riemann_zeta(decimal_type { 1 })));

      const decimal_type minus_half { -5, -1 };

      BOOST_TEST_EQ(riemann_zeta(zero), minus_half);
    }
  }
}

} // namespace local

int main()
{
    bool result_is_ok { true };

    {
        using decimal_type = ::boost::decimal::decimal32;

        const bool result_rz32_is_ok = local::test_riemann_zeta<decimal_type, float>(2048);

        result_is_ok = (result_rz32_is_ok && result_is_ok);

        BOOST_TEST(result_is_ok);
    }

    {
        using decimal_type = ::boost::decimal::decimal64;

        const bool result_rz64_is_ok = local::test_riemann_zeta<decimal_type, double>(8192);

        result_is_ok = (result_rz64_is_ok && result_is_ok);

        BOOST_TEST(result_is_ok);
    }

    {
        using decimal_type = ::boost::decimal::decimal32;

        local::test_riemann_zeta_edge<decimal_type, float>();

        BOOST_TEST(result_is_ok);
    }

    static_cast<void>(result_is_ok);

    return boost::report_errors();
}
