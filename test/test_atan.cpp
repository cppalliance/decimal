// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Propogates up from boost.math
#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/math/special_functions/next.hpp>
#include <iostream>
#include <random>
#include <cmath>

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(128U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(128U >> 4U); // Number of trials
#endif

static std::mt19937_64 rng(42);

using namespace boost::decimal;

template <typename Dec>
void test_atan()
{
    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};

    for (std::size_t n {}; n < max_iter; ++n)
    {
        std::uniform_real_distribution<float> small_vals(0.0F, 0.4375F);
        const auto val1 {small_vals(rng)};
        Dec d1 {val1};

        auto ret_val {std::atan(val1)};
        auto ret_dec {static_cast<float>(atan(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 100))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
        }
    }

    for (std::size_t n {}; n < max_iter; ++n)
    {
        std::uniform_real_distribution<float> half_vals(0.4375F, 0.6875F);
        const auto val1 {half_vals(rng)};
        Dec d1 {val1};

        auto ret_val {std::atan(val1)};
        auto ret_dec {static_cast<float>(atan(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 100))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
        }
    }

    for (std::size_t n {}; n < max_iter; ++n)
    {
        std::uniform_real_distribution<float> one_vals(0.6875F, 1.1875F);
        const auto val1 {one_vals(rng)};
        Dec d1 {val1};

        auto ret_val {std::atan(val1)};
        auto ret_dec {static_cast<float>(atan(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 100))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
        }
    }

    for (std::size_t n {}; n < max_iter; ++n)
    {
        std::uniform_real_distribution<float> three_halves(1.1875F, 2.4375F);
        const auto val1 {three_halves(rng)};
        Dec d1 {val1};

        auto ret_val {std::atan(val1)};
        auto ret_dec {static_cast<float>(atan(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 100))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
        }
    }

    for (std::size_t n {}; n < max_iter; ++n)
    {
        std::uniform_real_distribution<float> big_vals(2.4375F, 100.0F);
        const auto val1 {big_vals(rng)};
        Dec d1 {val1};

        auto ret_val {std::atan(val1)};
        auto ret_dec {static_cast<float>(atan(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 1e6))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
        }
    }

    // Edge cases
    std::uniform_int_distribution<int> one(1,1);
    BOOST_TEST_EQ(atan(std::numeric_limits<Dec>::infinity() * Dec(one(rng))), numbers::pi_v<Dec>/2);
    BOOST_TEST_EQ(atan(-std::numeric_limits<Dec>::infinity() * Dec(one(rng))), -numbers::pi_v<Dec>/2);
    BOOST_TEST(isnan(atan(std::numeric_limits<Dec>::quiet_NaN() * Dec(one(rng)))));
    BOOST_TEST_EQ(atan(Dec(0) * Dec(one(rng))), Dec(0));
    BOOST_TEST_EQ(atan(std::numeric_limits<Dec>::epsilon() * Dec(one(rng))), std::numeric_limits<Dec>::epsilon() * Dec(one(rng)));
}

int main()
{
    test_atan<decimal32>();
    test_atan<decimal64>();

    return boost::report_errors();
}
