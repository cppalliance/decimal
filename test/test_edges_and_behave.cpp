// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <chrono>
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
  auto is_close_fraction(const NumericType a,
                         const NumericType b,
                         const NumericType tol) noexcept -> bool
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

  auto test_behave_over_under() -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    auto result_is_ok = true;

    {
      const auto local_nan_to_construct_f  = decimal_type { std::numeric_limits<float>::quiet_NaN() };
      const auto local_nan_to_construct_d  = decimal_type { std::numeric_limits<double>::quiet_NaN() };
      const auto local_nan_to_construct_ld = decimal_type { std::numeric_limits<long double>::quiet_NaN() };

      const auto result_nan_construct_is_ok =
        (
             isnan(local_nan_to_construct_f)
          && isnan(local_nan_to_construct_d)
          && isnan(local_nan_to_construct_ld)
        );

      BOOST_TEST(result_nan_construct_is_ok);

      result_is_ok = (result_nan_construct_is_ok && result_is_ok);

      {
        const auto sum_nan_01 = local_nan_to_construct_f + 1;
        const auto sum_nan_02 = local_nan_to_construct_f + decimal_type { 2, 0 };
        const auto sum_nan_03 = local_nan_to_construct_f + decimal_type { 3.0L };

        const auto result_sum_nan_is_ok = (isnan(sum_nan_01) && isnan(sum_nan_02) && isnan(sum_nan_03));

        BOOST_TEST(result_sum_nan_is_ok);

        result_is_ok = (result_sum_nan_is_ok && result_is_ok);
      }

      {
        const auto dif_nan_01 = local_nan_to_construct_f - 1;
        const auto dif_nan_02 = local_nan_to_construct_f - decimal_type { 2, 0 };
        const auto dif_nan_03 = local_nan_to_construct_f - decimal_type { 3.0L };

        const auto result_dif_nan_is_ok = (isnan(dif_nan_01) && isnan(dif_nan_02) && isnan(dif_nan_03));

        BOOST_TEST(result_dif_nan_is_ok);

        result_is_ok = (result_dif_nan_is_ok && result_is_ok);
      }
    }

    {
      decimal_type big {2, 0};

      for(auto   index = static_cast<std::size_t>(UINT8_C(0));
                 index < static_cast<std::size_t>(UINT16_C(1000));
               ++index)
      {
        static_cast<void>(index);

        big *= big;
      }

      const auto result_big_inf_is_ok = isinf(big);

      BOOST_TEST(result_big_inf_is_ok);

      result_is_ok = (result_big_inf_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

  auto test_eges() -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    auto result_is_ok = true;

    {
      const decimal_type a { 1.234567e5L };
      const decimal_type b { 9.876543e-2L };

      const decimal_type c = a + b;

      const auto result_prec_add_is_ok = (c == decimal_type(123456.709876543L));

      BOOST_TEST(result_prec_add_is_ok);

      result_is_ok = (result_prec_add_is_ok && result_is_ok);
    }

    {
      std::random_device rd;
      std::mt19937_64 gen(rd());

      gen.seed(static_cast<typename std::mt19937_64::result_type>(UINT64_C(0x12345678AA55)));

      for(auto   index = static_cast<unsigned>(UINT8_C(0));
                 index < static_cast<unsigned>(UINT8_C(1000));
               ++index)
      {
        auto dis_lhs =
          std::uniform_real_distribution<float>
          {
            static_cast<float>(1.0E-5L),
            static_cast<float>(2.0E+5L)
          };

        auto dis_rhs =
          std::uniform_real_distribution<float>
          {
            static_cast<float>(8.0E-2L),
            static_cast<float>(11.0E-2L)
          };

        const auto lhs_flt = dis_lhs(gen);
        const auto rhs_flt = dis_rhs(gen);

        const auto lhs_dec = decimal_type(lhs_flt);
        const auto rhs_dec = decimal_type(rhs_flt);

        const auto sum_dec = lhs_dec + rhs_dec;
        const auto sum_flt = lhs_flt + rhs_flt;

        const auto result_sum_is_ok = is_close_fraction(sum_flt, static_cast<float>(sum_dec), std::numeric_limits<float>::epsilon() * 16);

        BOOST_TEST(result_sum_is_ok);

        result_is_ok = (result_sum_is_ok && result_is_ok);
      }
    }

    return result_is_ok;
  }
}

auto main() -> int
{
  auto result_is_ok = (local::test_behave_over_under() && local::test_eges());

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
