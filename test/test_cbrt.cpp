// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

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

#include <boost/math/special_functions/next.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <random>
#include <type_traits>

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH) && !defined(_WIN32)
static constexpr auto N = static_cast<std::size_t>(128U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(128U >> 4U); // Number of trials
#endif

static std::mt19937_64 rng(42);

namespace local {

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
} // namespace local

using namespace boost::decimal;

template <typename Dec>
void test_random_cbrt(const int tol_factor)
{
    using comp_type = std::conditional_t<std::is_same<Dec, decimal32>::value, float, double>;
    std::uniform_real_distribution<comp_type> dist(1, 1e3);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::cbrt(val1)};
        auto ret_dec {static_cast<comp_type>(cbrt(d1))};

        const auto result_val_is_ok = local::is_close_fraction(ret_val, static_cast<comp_type>(ret_dec), static_cast<comp_type>(std::numeric_limits<Dec>::epsilon()) * static_cast<comp_type>(tol_factor));

        if (!BOOST_TEST(result_val_is_ok))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << boost::math::float_distance(ret_val, ret_dec) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    std::uniform_real_distribution<comp_type> small_dist(0, 1);

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {small_dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::cbrt(val1)};
        auto ret_dec {static_cast<comp_type>(cbrt(d1))};

        if (!BOOST_TEST(std::abs(boost::math::float_distance(ret_val, ret_dec)) < 15))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << boost::math::float_distance(ret_val, ret_dec) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    Dec inf {std::numeric_limits<Dec>::infinity() * static_cast<int>(dist(rng))};
    Dec nan {std::numeric_limits<Dec>::quiet_NaN() * static_cast<int>(dist(rng))};
    Dec zero {0 * static_cast<int>(dist(rng))};
    Dec neg_num {-static_cast<int>(dist(rng))};
    BOOST_TEST(isinf(cbrt(inf)));
    BOOST_TEST(isnan(cbrt(-inf)));
    BOOST_TEST(isnan(cbrt(nan)));
    BOOST_TEST(isnan(cbrt(-nan)));
    BOOST_TEST_EQ(cbrt(zero), zero);
    BOOST_TEST(isnan(cbrt(neg_num)));
}

template <typename T>
void test_spot(T val, T expected_result)
{
    using comp_type = std::conditional_t<std::is_same<T, decimal32>::value, float, double>;

    const T val_cbrt {cbrt(val)};

    if (!BOOST_TEST(std::abs(boost::math::float_distance(static_cast<comp_type>(val_cbrt), static_cast<comp_type>(expected_result))) < 15))
    {
        // LCOV_EXCL_START
        std::cerr << "   Val: " << val
                  << "\n  Cbrt: " << val_cbrt
                  << "\nExpect: " << expected_result
                  << "\nEps: " << boost::math::float_distance(static_cast<comp_type>(val_cbrt), static_cast<comp_type>(expected_result)) << std::endl;
        // LCOV_EXCL_STOP
    }
}

// https://github.com/cppalliance/decimal/issues/440
template <typename T>
void test_spots()
{
    test_spot(T{8}, T{2});
    test_spot(T{27}, T{3});
    test_spot(T{64}, T{4});
    test_spot(T{125}, T{5});
    test_spot(T{216}, T{6});
}

namespace local {

  template<typename DecimalType>
  auto test_cbrt_edge() -> bool
  {
    using decimal_type = DecimalType;

    auto result_is_ok = true;

    {
      int np = -33;

      for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(23)); ++i)
      {
        static_cast<void>(i);

        const decimal_type arg_p10 { 1, np };

        const decimal_type val_p10 = cbrt(arg_p10);

        const auto result_val_p10_is_ok = val_p10 == decimal_type { 1, np / 3 };

        np += 3;

        BOOST_TEST(result_val_p10_is_ok);

        result_is_ok = (result_val_p10_is_ok && result_is_ok);
      }
    }

    return result_is_ok;
  }

} // namespace local

int main()
{
    test_random_cbrt<decimal32>(32);
    test_random_cbrt<decimal64>(32);

    test_spots<decimal32>();
    test_spots<decimal64>();

    #ifndef BOOST_DECIMAL_REDUCE_TEST_DEPTH
    test_random_cbrt<decimal128>(128);
    test_spots<decimal128>();
    #endif

    local::test_cbrt_edge<decimal32>();
    local::test_cbrt_edge<decimal64>();
    local::test_cbrt_edge<decimal128>();

    return boost::report_errors();
}
