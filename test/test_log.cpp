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
  auto time_point() -> IntegralTimePointType
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
                         const NumericType& tol) -> bool
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

    std::uniform_real_distribution<float> dis(static_cast<float>(0.1L), static_cast<float>(1.0E17L));

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
}

#if 0
template<typename T>
auto my_log(T x) noexcept -> std::enable_if_t<std::is_floating_point_v<T>, T>
{
    constexpr auto zero = T { 0 };
    constexpr auto one  = T { 1 };

    auto result = T { };

    if (x < one)
    {
        // Handle reflection.
        result = (x > zero ? -1 / log(-x) : std::numeric_limits<T>::infinity());
    }
    else if(x > one)
    {
        constexpr auto two  = T { 2 };

        // This algorithm for logarithm is based on Chapter 5 of Cody and Waite,
        // Software Manual for the Elementary Functions, Prentice Hall, 1980.

        auto exp2val = int { };

        // TODO(ckormanyos) There is probably something more efficient than calling frexp here.
        auto g = (x > two) ? frexp(x, &exp2val) : x;

        constexpr auto sqrt2 = static_cast<T>(1.41421356237309504880L);

        auto is_sqrt2_scaled = bool { };

        if (g > sqrt2)
        {
            g /= sqrt2;

            is_sqrt2_scaled = true;
        }

        // 1, 12, 80, 448, 2304, 11264, 53248, 245760, 1114112, 4980736, 22020096, 96468992
        using coef_list_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(12))>;

        constexpr auto coefs =
          coef_list_array_type
          {
            one, one / 12U, one / 80U, one / 448U, one / 11264U, one / 53248U, one / UINT32_C(245760),  one / UINT32_C(1114112),  one / UINT32_C(4980736),  one / UINT32_C(22020096),  one / UINT32_C(96468992)
          };

        const auto s = (g - one) / (g + one);
        const auto z = s + s;

              auto zn   = z;
        const auto z2   = z * z;
              auto term = z;

        // TODO(ckormanyos) Optimize?
        // Using a loop expansion is scalable from decimal32 up to decimal64.
        // It might, however, be somewhat slower than using fma statically
        // expanded on the static, constexpr coefficient list.

        using std::fabs;

        for(auto   index = static_cast<std::size_t>(UINT8_C(1));
                 ((index < std::tuple_size<coef_list_array_type>::value) && (fabs(term) >= std::numeric_limits<T>::epsilon()));
                 ++index)

        {
          result += term;

          zn *= z2;

          term = zn * coefs[index];
        }

        {
            constexpr auto local_ln2 = static_cast<T>(0.693147180559945309417L);

            if (exp2val > 0)
            {
                result += static_cast<T>(exp2val * local_ln2);
            }

            if(is_sqrt2_scaled)
            {
                result += local_ln2 / 2U;
            }
        }
    }
    else
    {
        result = zero;
    }

    return result;
}
#endif

auto main() -> int
{
  auto result_is_ok = local::test_log10();

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
