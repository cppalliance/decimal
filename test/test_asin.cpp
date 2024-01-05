// Copyright 2024 Matt Borland
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
void test_asin()
{
    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};

    for (std::size_t n {}; n < max_iter; ++n)
    {
        std::uniform_real_distribution<float> small_vals(0.0F, 0.5F);
        const auto val1 {small_vals(rng)};
        Dec d1 {val1};

        auto ret_val {std::asin(val1)};
        auto ret_dec {static_cast<float>(asin(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 50))
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
        std::uniform_real_distribution<float> big_vals(0.5F, 0.9999F);
        const auto val1 {big_vals(rng)};
        Dec d1 {val1};

        auto ret_val {std::asin(val1)};
        auto ret_dec {static_cast<float>(asin(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 50))
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
        std::uniform_real_distribution<float> neg_vals(-0.9999F, 0.0F);
        const auto val1 {neg_vals(rng)};
        Dec d1 {val1};

        auto ret_val {std::asin(val1)};
        auto ret_dec {static_cast<float>(asin(d1))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 50))
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
    BOOST_TEST(isnan(asin(std::numeric_limits<Dec>::infinity() * Dec(one(rng)))));
    BOOST_TEST(isnan(asin(-std::numeric_limits<Dec>::infinity() * Dec(one(rng)))));
    BOOST_TEST(isnan(asin(std::numeric_limits<Dec>::quiet_NaN() * Dec(one(rng)))));
    BOOST_TEST_EQ(asin(std::numeric_limits<Dec>::epsilon() * Dec(one(rng))), std::numeric_limits<Dec>::epsilon() * Dec(one(rng)));
}

int main()
{
    test_asin<decimal32>();
    test_asin<decimal64>();

    return boost::report_errors();
}
