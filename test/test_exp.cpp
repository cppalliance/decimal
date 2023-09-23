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

auto my_zero() -> boost::decimal::decimal32&;
auto my_one () -> boost::decimal::decimal32&;

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

  auto test_exp(const int tol_factor, const bool negate) -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    std::random_device rd;
    std::mt19937_64 gen(rd());

    gen.seed(time_point<typename std::mt19937_64::result_type>());

    auto dis =
      std::uniform_real_distribution<float>
      {
        static_cast<float>(0.03125L),
        static_cast<float>(32.0L)
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
      const auto x_flt_begin = dis(gen);

      const auto x_flt = (negate ? -x_flt_begin : x_flt_begin);
      const auto x_dec = static_cast<decimal_type>(x_flt);

      using std::exp;

      const auto exp_flt = exp(x_flt);
      const auto exp_dec = exp(x_dec);

      const auto result_log_is_ok = is_close_fraction(exp_flt, static_cast<float>(exp_dec), std::numeric_limits<float>::epsilon() * tol_factor);

      result_is_ok = (result_log_is_ok && result_is_ok);

      if(!result_log_is_ok)
      {
        std::cout << "x_flt  : " <<                    x_flt   << std::endl;
        std::cout << "exp_flt: " << std::scientific << exp_flt << std::endl;
        std::cout << "exp_dec: " << std::scientific << exp_dec << std::endl;

        break;
      }
    }

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  auto test_exp_edge() -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    std::mt19937_64 gen;

    std::uniform_real_distribution<float> dist(1.01F, 1.04F);

    auto result_is_ok = true;

    {
      const auto exp_nan = exp(std::numeric_limits<decimal_type>::quiet_NaN() * static_cast<decimal_type>(dist(gen)));

      const auto result_exp_nan_is_ok = isnan(exp_nan);

      BOOST_TEST(result_exp_nan_is_ok);

      result_is_ok = (result_exp_nan_is_ok && result_is_ok);
    }

    {
      const auto exp_inf_pos = exp(std::numeric_limits<decimal_type>::infinity() * static_cast<decimal_type>(dist(gen)));

      const auto result_exp_inf_pos_is_ok = isinf(exp_inf_pos);

      BOOST_TEST(result_exp_inf_pos_is_ok);

      result_is_ok = (result_exp_inf_pos_is_ok && result_is_ok);
    }

    {
      const auto exp_inf_neg = exp(-std::numeric_limits<decimal_type>::infinity() * static_cast<decimal_type>(dist(gen)));

      const auto result_exp_inf_neg_is_ok = (exp_inf_neg == ::my_zero());

      BOOST_TEST(result_exp_inf_neg_is_ok);

      result_is_ok = (result_exp_inf_neg_is_ok && result_is_ok);
    }

    {
      const auto exp_zero_pos = exp(::my_zero());

      const auto result_exp_zero_pos_is_ok = (exp_zero_pos == ::my_one());

      BOOST_TEST(result_exp_zero_pos_is_ok);

      result_is_ok = (result_exp_zero_pos_is_ok && result_is_ok);
    }

    {
      const auto exp_zero_neg = exp(-::my_zero());

      const auto result_exp_zero_neg_is_ok = (exp_zero_neg == ::my_one());

      BOOST_TEST(result_exp_zero_neg_is_ok);

      result_is_ok = (result_exp_zero_neg_is_ok && result_is_ok);
    }

    return result_is_ok;
  }

} // namespace local

auto main() -> int
{
  auto result_is_ok = true;

  const auto result_exp_pos_is_ok = local::test_exp(96, false);
  const auto result_exp_neg_is_ok = local::test_exp(96, true);
  const auto result_exp_edge_is_ok = local::test_exp_edge();

  result_is_ok = (result_exp_pos_is_ok  && result_is_ok);
  result_is_ok = (result_exp_neg_is_ok  && result_is_ok);
  result_is_ok = (result_exp_edge_is_ok && result_is_ok);

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}

auto my_zero() -> boost::decimal::decimal32& { static boost::decimal::decimal32 val_zero { 0, 0 }; return val_zero; }
auto my_one () -> boost::decimal::decimal32& { static boost::decimal::decimal32 val_one  { 1, 0 }; return val_one; }
