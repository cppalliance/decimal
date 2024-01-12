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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge cases
    std::uniform_int_distribution<int> one(1,1);
    BOOST_TEST(isnan(asin(std::numeric_limits<Dec>::infinity() * Dec(one(rng)))));
    BOOST_TEST(isnan(asin(-std::numeric_limits<Dec>::infinity() * Dec(one(rng)))));
    BOOST_TEST(isnan(asin(std::numeric_limits<Dec>::quiet_NaN() * Dec(one(rng)))));
    BOOST_TEST_EQ(asin(std::numeric_limits<Dec>::epsilon() * Dec(one(rng))), std::numeric_limits<Dec>::epsilon() * Dec(one(rng)));
}

template <typename T>
void print_value(T value, const char* str)
{
    int ptr;
    const auto sig_val = frexp10(value, &ptr);
    std::cerr << std::setprecision(std::numeric_limits<T>::digits10) << str << ": " << value
              << "\nSig: " << sig_val.high << " " << sig_val.low
              << "\nExp: " << ptr << "\n" << std::endl;
}

int main()
{
    test_asin<decimal32>();
    test_asin<decimal64>();

    #ifdef BOOST_DECIMAL_GENERATE_CONSTANT_SIGS
    print_value("0.2638070997559249856723985171001241"_DL, "a0");
    print_value("-0.434913932128320201763937904114753451"_DL, "a1");
    print_value("0.305590847861021053826474613710909921"_DL, "a2");
    print_value("-0.13977130653211100015879395629540542"_DL, "a3");
    print_value("0.054573213517731915574990756064112231"_DL, "a4");
    print_value("0.064051743877986184818706014119123069"_DL, "a5");
    print_value("0.0011606701725692841523963098473051048"_DL, "a6");
    print_value("0.16650989049586517768081355278405235"_DL, "a7");
    print_value("2.5906093603606160032693191317527350e-06"_DL, "a8");
    print_value("0.99999996600620587094099811028495259"_DL, "a9");
    print_value("7.3651618060008750986002928690410142e-11"_DL, "a10");

    throw;
    #endif

    return boost::report_errors();
}
