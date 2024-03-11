// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <boost/math/special_functions/next.hpp>

#include <iostream>
#include <random>
#include <type_traits>

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH) && !defined(_WIN32)
static constexpr auto N = static_cast<std::size_t>(128U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(128U >> 4U); // Number of trials
#endif

static std::mt19937_64 rng(42);

using namespace boost::decimal;

template <typename Dec>
void test_random_cbrt()
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

int main()
{
    test_random_cbrt<decimal32>();
    test_random_cbrt<decimal64>();

    test_spots<decimal32>();
    test_spots<decimal64>();

    #ifndef BOOST_DECIMAL_REDUCE_TEST_DEPTH
    test_random_cbrt<decimal128>();
    test_spots<decimal128>();
    #endif

    return boost::report_errors();
}
