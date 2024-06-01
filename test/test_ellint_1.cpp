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

#include <boost/math/special_functions/ellint_1.hpp>
#include <boost/math/special_functions/next.hpp>
#include <boost/core/lightweight_test.hpp>

#include <iostream>
#include <random>
#include <cmath>

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
    std::uniform_real_distribution<float> dist(-0.9F, 0.9F);

    constexpr auto local_N = std::is_same<T, decimal128>::value ? N / 4 : N;
    for (std::size_t i {}; i < local_N; ++i)
    {
        const auto val {dist(rng)};
        const T dec_val {val};

        const auto float_res {boost::math::ellint_1(val)};
        const auto dec_res {static_cast<float>(comp_ellint_1(dec_val))};
        const auto distance {boost::math::float_distance(float_res, dec_res)};

        if (!BOOST_TEST(std::abs(distance) < 32))
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
    std::uniform_real_distribution<float> dist_k  (-0.9F, 0.9F);
    std::uniform_real_distribution<float> dist_phi(-0.9F, 0.9F);

    constexpr auto local_N = std::is_same<T, decimal128>::value ? N / 4 : N;
    for (std::size_t i {}; i < local_N; ++i)
    {
        float phi_val_try { };

        using std::fabs;

        do
        {
          phi_val_try = dist_phi(rng);
        }
        while(fabs(phi_val_try) < 0.1F);

        const auto k_val {dist_k(rng)};
        const auto phi_val {phi_val_try};
        const T k_dec_val {k_val};
        const T phi_dec_val {phi_val};

        const auto float_res {boost::math::ellint_1(k_val, phi_val)};
        const auto dec_res {static_cast<float>(ellint_1(k_dec_val, phi_dec_val))};
        const auto distance {boost::math::float_distance(float_res, dec_res)};

        if (!BOOST_TEST(std::abs(distance) < 128))
        {
            // LCOV_EXCL_START
            std::cerr << "Float: " << float_res
                      << "\n  Dec: " << dec_res
                      << "\n Dist: " << distance << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

int main()
{
    test_comp_ellint<decimal32>();
    test_comp_ellint<decimal64>();

    test_ellint<decimal32>();
    test_ellint<decimal64>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test_comp_ellint<decimal128>();
    //test_ellint<decimal128>();
    #endif

    return boost::report_errors();
}
