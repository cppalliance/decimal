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
static constexpr auto N = static_cast<std::size_t>(512U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(512U >> 4U); // Number of trials
#endif

static std::mt19937_64 rng(42);

using namespace boost::decimal;

template <typename Dec>
void test_fmax()
{
    
    std::uniform_int_distribution<int> dist(0, 10);

    BOOST_TEST_EQ(fmax(Dec(1), std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng))), Dec(1));
    BOOST_TEST_EQ(fmax(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), Dec(1)), Dec(1));
    BOOST_TEST(isnan(fmax(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST_EQ(fmax(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)), -std::numeric_limits<Dec>::infinity() * Dec(dist(rng))), std::numeric_limits<Dec>::infinity());

    BOOST_TEST_EQ(fmax(Dec(1), Dec(0)), Dec(1));
    BOOST_TEST_EQ(fmax(Dec(-2), Dec(1)), Dec(1));

    // Mixed types
    BOOST_TEST_EQ(fmax(decimal128(1), Dec(0)), decimal128(1));
    BOOST_TEST_EQ(fmax(decimal128(-2), Dec(1)), decimal128(1));
}

template <typename Dec>
void test_isgreater()
{
    BOOST_TEST_EQ(isgreater(Dec(1), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(isgreater(std::numeric_limits<Dec>::quiet_NaN(), Dec(1)), false);
    BOOST_TEST_EQ(isgreater(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(isgreater(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), true);

    BOOST_TEST_EQ(isgreater(Dec(1), Dec(0)), true);
    BOOST_TEST_EQ(isgreater(Dec(-2), Dec(1)), false);
    BOOST_TEST_EQ(isgreater(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_isgreaterequal()
{
    BOOST_TEST_EQ(isgreaterequal(Dec(1), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(isgreaterequal(std::numeric_limits<Dec>::quiet_NaN(), Dec(1)), false);
    BOOST_TEST_EQ(isgreaterequal(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(isgreaterequal(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), true);

    BOOST_TEST_EQ(isgreaterequal(Dec(1), Dec(0)), true);
    BOOST_TEST_EQ(isgreaterequal(Dec(-2), Dec(1)), false);
    BOOST_TEST_EQ(isgreaterequal(Dec(1), Dec(1)), true);
}

template <typename Dec>
void test_fmin()
{
    BOOST_TEST_EQ(fmin(Dec(1), std::numeric_limits<Dec>::quiet_NaN()), Dec(1));
    BOOST_TEST_EQ(fmin(std::numeric_limits<Dec>::quiet_NaN(), Dec(1)), Dec(1));
    BOOST_TEST(isnan(fmin(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN())));
    BOOST_TEST_EQ(fmin(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), -std::numeric_limits<Dec>::infinity());

    BOOST_TEST_EQ(fmin(Dec(1), Dec(0)), Dec(0));
    BOOST_TEST_EQ(fmin(Dec(-2), Dec(1)), Dec(-2));

    // Mixed types
    BOOST_TEST_EQ(fmin(decimal128(1), Dec(0)), decimal128(0));
    BOOST_TEST_EQ(fmin(decimal128(-2), Dec(1)), decimal128(-2));
}

template <typename Dec>
void test_isless()
{
    BOOST_TEST_EQ(isless(Dec(1), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(isless(std::numeric_limits<Dec>::quiet_NaN(), Dec(1)), false);
    BOOST_TEST_EQ(isless(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(isless(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), false);

    BOOST_TEST_EQ(isless(Dec(1), Dec(0)), false);
    BOOST_TEST_EQ(isless(Dec(-2), Dec(1)), true);
    BOOST_TEST_EQ(isless(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_islessequal()
{
    BOOST_TEST_EQ(islessequal(Dec(1), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(islessequal(std::numeric_limits<Dec>::quiet_NaN(), Dec(1)), false);
    BOOST_TEST_EQ(islessequal(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(islessequal(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), false);

    BOOST_TEST_EQ(islessequal(Dec(1), Dec(0)), false);
    BOOST_TEST_EQ(islessequal(Dec(-2), Dec(1)), true);
    BOOST_TEST_EQ(islessequal(Dec(1), Dec(1)), true);
}

template <typename Dec>
void test_islessgreater()
{
    BOOST_TEST_EQ(islessgreater(Dec(1), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(islessgreater(std::numeric_limits<Dec>::quiet_NaN(), Dec(1)), false);
    BOOST_TEST_EQ(islessgreater(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN()), false);
    BOOST_TEST_EQ(islessgreater(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), true);

    BOOST_TEST_EQ(islessgreater(Dec(1), Dec(0)), true);
    BOOST_TEST_EQ(islessgreater(Dec(-2), Dec(1)), true);
    BOOST_TEST_EQ(islessgreater(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_isunordered()
{
    BOOST_TEST_EQ(isunordered(Dec(1), std::numeric_limits<Dec>::quiet_NaN()), true);
    BOOST_TEST_EQ(isunordered(std::numeric_limits<Dec>::quiet_NaN(), Dec(1)), true);
    BOOST_TEST_EQ(isunordered(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN()), true);
    BOOST_TEST_EQ(isunordered(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), false);

    BOOST_TEST_EQ(isunordered(Dec(1), Dec(0)), false);
    BOOST_TEST_EQ(isunordered(Dec(-2), Dec(1)), false);
    BOOST_TEST_EQ(isunordered(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_floor()
{
    
    std::uniform_int_distribution<int> dist(1, 1);

    BOOST_TEST(isnan(floor(std::numeric_limits<Dec>::quiet_NaN() * dist(rng))));
    BOOST_TEST(isnan(floor(-std::numeric_limits<Dec>::quiet_NaN() * dist(rng))));
    BOOST_TEST(isinf(floor(std::numeric_limits<Dec>::infinity() * dist(rng))));
    BOOST_TEST(isinf(floor(-std::numeric_limits<Dec>::infinity() * dist(rng))));
    BOOST_TEST_EQ(floor(Dec(0, 0) * dist(rng)), Dec(0, 0));
    BOOST_TEST_EQ(floor(Dec(-0, 0) * dist(rng)), Dec(-0, 0));

    BOOST_TEST_EQ(floor(Dec(27, -1) * dist(rng)), Dec(2, 0));
    BOOST_TEST_EQ(floor(Dec(-27, -1) * dist(rng)), Dec(-3, 0));
    BOOST_TEST_EQ(floor(Dec(27777, -4) * dist(rng)), Dec(2, 0));
    BOOST_TEST_EQ(floor(Dec(-27777, -4) * dist(rng)), Dec(-3, 0));

    // Bigger numbers
    BOOST_TEST_EQ(floor(Dec(27777, -2) * dist(rng)), Dec(277, 0));
    BOOST_TEST_EQ(floor(Dec(-27777, -2) * dist(rng)), Dec(-277, 0));
    BOOST_TEST_EQ(floor(Dec(27777, -1) * dist(rng)), Dec(2777, 0));

    // Near zero
    BOOST_TEST_EQ(floor(Dec(3, -1) * dist(rng)), Dec(0, 0));
    BOOST_TEST_EQ(floor(Dec(-3, -1) * dist(rng)), Dec(-1, 0));
}

template <typename Dec>
void test_ceil()
{
    
    std::uniform_int_distribution<int> dist(0, 10);

    BOOST_TEST(isnan(ceil(std::numeric_limits<Dec>::quiet_NaN() * dist(rng))));
    BOOST_TEST(isnan(ceil(-std::numeric_limits<Dec>::quiet_NaN() * dist(rng))));
    BOOST_TEST(isinf(ceil(std::numeric_limits<Dec>::infinity() * dist(rng))));
    BOOST_TEST(isinf(ceil(-std::numeric_limits<Dec>::infinity() * dist(rng))));
    BOOST_TEST_EQ(ceil(Dec(0, 0) * dist(rng)), Dec(0, 0));
    BOOST_TEST_EQ(ceil(Dec(-0, 0) * dist(rng)), Dec(-0, 0));

    BOOST_TEST_EQ(ceil(Dec(27, -1)), Dec(3, 0));
    BOOST_TEST_EQ(ceil(Dec(-27, -1)), Dec(-2, 0));
    BOOST_TEST_EQ(ceil(Dec(27777, -4)), Dec(3, 0));
    BOOST_TEST_EQ(ceil(Dec(-27777, -4)), Dec(-2, 0));

    // Bigger numbers
    BOOST_TEST_EQ(ceil(Dec(27777, -2)), Dec(278, 0));
    BOOST_TEST_EQ(ceil(Dec(-27777, -2)), Dec(-277, 0));
    BOOST_TEST_EQ(ceil(Dec(27777, -1)), Dec(2778, 0));

    // Near zero
    BOOST_TEST_EQ(ceil(Dec(3, -1)), Dec(1, 0));
    BOOST_TEST_EQ(ceil(Dec(-3, -1)), Dec(0, 0));
}

template <typename Dec>
void test_trunc()
{
    BOOST_TEST(isnan(trunc(std::numeric_limits<Dec>::quiet_NaN())));
    BOOST_TEST(isnan(trunc(-std::numeric_limits<Dec>::quiet_NaN())));
    BOOST_TEST(isinf(trunc(std::numeric_limits<Dec>::infinity())));
    BOOST_TEST(isinf(trunc(-std::numeric_limits<Dec>::infinity())));
    BOOST_TEST_EQ(trunc(Dec(0, 0)), Dec(0, 0));
    BOOST_TEST_EQ(trunc(Dec(-0, 0)), Dec(-0, 0));

    BOOST_TEST_EQ(trunc(Dec(27, -1)), Dec(2, 0));
    BOOST_TEST_EQ(trunc(Dec(-27, -1)), Dec(-2, 0));
    BOOST_TEST_EQ(trunc(Dec(27777, -4)), Dec(2, 0));
    BOOST_TEST_EQ(trunc(Dec(-27777, -4)), Dec(-2, 0));
}

template <typename Dec>
void test_frexp10()
{
    auto expval = int {};

    BOOST_TEST_EQ(frexp10(Dec(0,0), &expval), 0);
    BOOST_TEST_EQ(expval, 0);

    expval = -1;
    BOOST_TEST_EQ(frexp10(std::numeric_limits<Dec>::quiet_NaN(), &expval), -1);
    BOOST_TEST_EQ(expval, 0);

    expval = -1;
    BOOST_TEST_EQ(frexp10(std::numeric_limits<Dec>::infinity(), &expval), -1);
    BOOST_TEST_EQ(expval, 0);

    BOOST_IF_CONSTEXPR (std::is_same<Dec, decimal32>::value)
    {
        BOOST_TEST_EQ(frexp10(Dec(10, 0), &expval), 1'000'000);
        BOOST_TEST_EQ(expval, -5);

        BOOST_TEST_EQ(frexp10(Dec(1'000'000, 5), &expval), 1'000'000);
        BOOST_TEST_EQ(expval, 5);

        BOOST_TEST_EQ(frexp10(Dec(-1'000'000, 5), &expval), 1'000'000);
        BOOST_TEST_EQ(expval, 5);
    }
    else BOOST_IF_CONSTEXPR (std::is_same<Dec, decimal64>::value)
    {
        BOOST_TEST_EQ(frexp10(Dec(10, 0), &expval), 1000000000000000);
        BOOST_TEST_EQ(expval, -14);

        BOOST_TEST_EQ(frexp10(Dec(1'000'000, 5), &expval), 1000000000000000);
        BOOST_TEST_EQ(expval, -4);

        BOOST_TEST_EQ(frexp10(Dec(-1'000'000, 5), &expval), 1000000000000000);
        BOOST_TEST_EQ(expval, -4);
    }
}

template <typename Dec>
void test_scalbn()
{
    BOOST_TEST(isinf(scalbn(std::numeric_limits<Dec>::infinity(), 1)));
    BOOST_TEST(isnan(scalbn(std::numeric_limits<Dec>::quiet_NaN(), 1)));
    BOOST_TEST_EQ(scalbn(Dec(0, 0), 1), Dec(0, 0));

    Dec one {1, 0};
    Dec ten {1, 1};
    Dec hundred {1, 2};

    BOOST_TEST_EQ(scalbn(one, 1), ten);
    BOOST_TEST_EQ(scalbn(one, 2), hundred);
    BOOST_TEST_EQ(scalbn(ten, 1), hundred);
    BOOST_TEST_EQ(scalbn(hundred, -1), ten);
    BOOST_TEST_EQ(scalbn(hundred, -2), one);
    BOOST_TEST_EQ(scalbn(hundred, 0), hundred);
    BOOST_TEST(isinf(scalbn(one, 10000)));
}

template <typename Dec>
void test_scalbln()
{
    BOOST_TEST(isinf(scalbln(std::numeric_limits<Dec>::infinity(), 1)));
    BOOST_TEST(isnan(scalbln(std::numeric_limits<Dec>::quiet_NaN(), 1)));
    BOOST_TEST_EQ(scalbln(Dec(0, 0), 1), Dec(0, 0));

    Dec one {1, 0};
    Dec ten {1, 1};
    Dec hundred {1, 2};

    BOOST_TEST_EQ(scalbln(one, 1), ten);
    BOOST_TEST_EQ(scalbln(one, 2), hundred);
    BOOST_TEST_EQ(scalbln(ten, 1), hundred);
    BOOST_TEST_EQ(scalbln(hundred, -1), ten);
    BOOST_TEST_EQ(scalbln(hundred, -2), one);
    BOOST_TEST_EQ(scalbln(hundred, 0), hundred);
    BOOST_TEST(isinf(scalbln(one, 10000)));
}

template <typename Dec>
void test_div_fmod()
{
    std::uniform_real_distribution<float> dist(0.0F, 1e30F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto float_div {val1 / val2};
        auto decimal_div {static_cast<float>(d1 / d2)};
        auto float_fmod {std::fmod(val1, val2)};
        auto decimal_fmod {static_cast<float>(fmod(d1, d2))};

        // Decimal types are all higher precision than float
        if (!(BOOST_TEST(std::fabs(boost::math::float_distance(float_fmod, decimal_fmod)) < 1e7) &&
              BOOST_TEST(std::fabs(boost::math::float_distance(float_div, decimal_div)) < 20)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nVal div: " << float_div
                      << "\nDec div: " << decimal_div
                      << "\nDist: " << boost::math::float_distance(float_div, decimal_div)
                      << "\nVal fmod: " << float_fmod
                      << "\nDec fmod: " << decimal_fmod
                      << "\nDist: " << boost::math::float_distance(float_fmod, decimal_fmod) << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename Dec>
void test_copysign()
{
    std::uniform_real_distribution<float> dist(0.0F, 1e30F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto ret_val {std::copysign(val1, val2)};
        auto ret_dec {static_cast<float>(copysign(d1, d2))};

        if (!BOOST_TEST(std::fabs(boost::math::float_distance(ret_val, ret_dec)) < 20))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename Dec>
void test_fma()
{
    if (!BOOST_TEST_EQ(Dec(1, -1) * Dec(1, 1), Dec(1, 0)))
    {
        // LCOV_EXCL_START
        std::cerr << std::setprecision(std::numeric_limits<Dec>::digits10)
                  << "   Mul: " << Dec(1, -1) * Dec(1, 1)
                  << "\nActual: " << Dec(1, 0) << std::endl;
        // LCOV_EXCL_STOP
    }

    BOOST_TEST_EQ(Dec(1, 0) + Dec(1, 0, true), Dec(0, 0));
    BOOST_TEST_EQ(fma(Dec(1, -1), Dec(1, 1), Dec(1, 0, true)), Dec(0, 0));

    std::uniform_real_distribution<double> dist(-1e10, 1e10);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        const auto val3 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};
        Dec d3 {val3};

        auto fma_val {fma(d1, d2, d3)};
        auto naive_val {(d1 * d2) + d3};

        if (!BOOST_TEST(fabs(fma_val - naive_val) < Dec(1, 5)))
        {
            // LCOV_EXCL_START
            std::cerr << "Dec 1: " << d1
                      << "\nDec 2: " << d2
                      << "\nDec 3: " << d3
                      << "\nfma val: " << fma_val
                      << "\nNaive val: " << naive_val
                      << "\nDist: " << fabs(fma_val - naive_val) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge cases
    BOOST_TEST(isinf(fma(std::numeric_limits<Dec>::infinity(), Dec(dist(rng)), Dec(dist(rng)))));
    BOOST_TEST(isnan(fma(std::numeric_limits<Dec>::quiet_NaN(), Dec(dist(rng)), Dec(dist(rng)))));
    BOOST_TEST(isinf(fma(Dec(dist(rng)), std::numeric_limits<Dec>::infinity(), Dec(dist(rng)))));
    BOOST_TEST(isnan(fma(Dec(dist(rng)), std::numeric_limits<Dec>::quiet_NaN(), Dec(dist(rng)))));
    BOOST_TEST(isinf(fma(Dec(dist(rng)), Dec(dist(rng)), std::numeric_limits<Dec>::infinity())));
    BOOST_TEST(isnan(fma(Dec(dist(rng)), Dec(dist(rng)), std::numeric_limits<Dec>::quiet_NaN())));
}

template <typename Dec>
void test_sin()
{
    std::uniform_real_distribution<float> dist(-3.14F * 2, 3.14F * 2);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::sin(val1)};
        auto ret_dec {static_cast<float>(sin(d1))};

        if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 30*std::numeric_limits<float>::epsilon()))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(sin(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)))));
    BOOST_TEST(isnan(sin(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST_EQ(sin(Dec(0) * Dec(dist(rng))), Dec(0));

    // Check the phases of large positive/negative arguments.
    using std::atan;

    for(auto x = 0.1F; x < 20.0F; x += 2.0F * atan(1.0F))
    {
      using std::sin;

      BOOST_TEST_EQ((sin(boost::decimal::decimal32 { x }) < 0), (sin(x) < 0));
    }

    for(auto x = 0.1F; x < 20.0F; x += 2.0F * atan(1.0F))
    {
      using std::sin;

      BOOST_TEST_EQ((sin(boost::decimal::decimal32 { -x }) < 0), (sin(-x) < 0));
    }
}

template <typename Dec>
void test_cos()
{
    std::uniform_real_distribution<float> dist(-3.14F * 2, 3.14F * 2);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::cos(val1)};
        auto ret_dec {static_cast<float>(cos(d1))};

        if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 25*std::numeric_limits<float>::epsilon()))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(cos(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)))));
    BOOST_TEST(isnan(cos(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST_EQ(cos(Dec(0) * Dec(dist(rng))), Dec(1));

    // Check the phases of large positive/negative arguments.
    using std::atan;

    for(auto x = 0.1F; x < 20.0F; x += 2.0F * atan(1.0F))
    {
      using std::cos;

      BOOST_TEST_EQ((cos(boost::decimal::decimal32 { x }) < 0), (cos(x) < 0));
    }

    for(auto x = 0.1F; x < 20.0F; x += 2.0F * atan(1.0F))
    {
      using std::cos;

      BOOST_TEST_EQ((cos(boost::decimal::decimal32 { -x }) < 0), (cos(-x) < 0));
    }
}

template <typename Dec>
void test_modf()
{
    Dec ptr {};
    BOOST_TEST_EQ(modf(Dec(123.45), &ptr), Dec(0.45));
    BOOST_TEST_EQ(ptr, Dec(123));

    BOOST_TEST_EQ(modf(std::numeric_limits<Dec>::infinity(), &ptr), Dec(0));
    BOOST_TEST_EQ(ptr, std::numeric_limits<Dec>::infinity());

    BOOST_TEST(modf(std::numeric_limits<Dec>::quiet_NaN(), &ptr) != std::numeric_limits<Dec>::quiet_NaN());
    BOOST_TEST(ptr != std::numeric_limits<Dec>::quiet_NaN());
}

template <typename Dec>
void test_remainder()
{
    std::uniform_real_distribution<float> dist(-1e3F, 1e3F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto ret_val {std::remainder(val1, val2)};
        auto ret_dec {static_cast<float>(remainder(d1, d2))};

        if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 0.005))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nFloat dist: " << boost::math::float_distance(ret_val, ret_dec) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isnan(remainder(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(remainder(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(remainder(Dec(1), std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST(isnan(remainder(Dec(1), Dec(0))));
}

template <typename Dec>
void test_remquo()
{
    std::uniform_real_distribution<float> dist(-1e3F, 1e3F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};
        int flt_int {};
        int dec_int {};

        auto ret_val {std::remquo(val1, val2, &flt_int)};
        auto ret_dec {static_cast<float>(remquo(d1, d2, &dec_int))};

        if ((!BOOST_TEST(std::fabs(ret_val - ret_dec) < 0.005) &&
             (BOOST_TEST(flt_int == dec_int))))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nInt val: " << flt_int
                      << "\nInt quo: " << val1 / val2
                      << "\nInt dec: " << dec_int
                      << "\nDec quo: " << d1 / d2
                      << "\nFloat dist: " << boost::math::float_distance(ret_val, ret_dec) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    int quo {};
    BOOST_TEST(isnan(remquo(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)), Dec(1), &quo)));
    BOOST_TEST(isnan(remquo(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), Dec(1), &quo)));
    BOOST_TEST(isnan(remquo(Dec(1), std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), &quo)));
    BOOST_TEST(isnan(remquo(Dec(1), Dec(0), &quo)));
}

template <typename Dec>
void test_fdim()
{
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto ret_val {std::fdim(val1, val2)};
        auto ret_dec {static_cast<float>(fdim(d1, d2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 1))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(fdim(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(fdim(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(fdim(Dec(1), std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST_EQ(fdim(Dec(1), Dec(1)), Dec(0));
}

template <typename Dec>
void test_ilogb()
{
    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<Dec, decimal32>::value)
    {
        BOOST_TEST_EQ(ilogb(Dec(1, 0)), 101);
        BOOST_TEST_EQ(ilogb(Dec(10, 0)), 102);
    }
    else BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<Dec, decimal64>::value)
    {
        BOOST_TEST_EQ(ilogb(Dec(1, 0)), 398);
        BOOST_TEST_EQ(ilogb(Dec(10, 0)), 399);
    }
    else
    {
        BOOST_TEST_EQ(ilogb(Dec(1, 0)), 6176);
        BOOST_TEST_EQ(ilogb(Dec(10, 0)), 6177);
    }

    BOOST_TEST_EQ(ilogb(Dec(0)), FP_ILOGB0);
    BOOST_TEST_EQ(ilogb(std::numeric_limits<Dec>::infinity()), INT_MAX);
    BOOST_TEST_EQ(ilogb(std::numeric_limits<Dec>::quiet_NaN()), FP_ILOGBNAN);
}

template <typename Dec>
void test_logb()
{
    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<Dec, decimal32>::value)
    {
        BOOST_TEST_EQ(ilogb(Dec(1, 0)), Dec(101));
        BOOST_TEST_EQ(ilogb(Dec(10, 0)), Dec(102));
    }
    else BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<Dec, decimal64>::value)
    {
        BOOST_TEST_EQ(ilogb(Dec(1, 0)), Dec(398));
        BOOST_TEST_EQ(ilogb(Dec(10, 0)), Dec(399));
    }
    else
    {
        BOOST_TEST_EQ(ilogb(Dec(1, 0)), 6176);
        BOOST_TEST_EQ(ilogb(Dec(10, 0)), 6177);
    }

    BOOST_TEST_EQ(logb(Dec(0)), -std::numeric_limits<Dec>::infinity());
    BOOST_TEST_EQ(logb(std::numeric_limits<Dec>::infinity()), std::numeric_limits<Dec>::infinity());
    BOOST_TEST(isnan(logb(std::numeric_limits<Dec>::quiet_NaN())));
}

template <typename Dec>
void test_sqrt()
{
    using comp_type = std::conditional_t<std::is_same<Dec, decimal32>::value, float, double>;
    std::uniform_real_distribution<comp_type> dist(0, 1e5);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::sqrt(val1)};
        auto ret_dec {static_cast<comp_type>(sqrt(d1))};

        if (!BOOST_TEST(boost::math::float_distance(ret_val, ret_dec) < 15))
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
    BOOST_TEST(isinf(sqrt(inf)));
    BOOST_TEST(isnan(sqrt(-inf)));
    BOOST_TEST(isnan(sqrt(nan)));
    BOOST_TEST(isnan(sqrt(-nan)));
    BOOST_TEST_EQ(sqrt(zero), zero);
    BOOST_TEST(isnan(sqrt(neg_num)));
}

template <typename Dec>
void test_cbrt()
{
    using comp_type = std::conditional_t<std::is_same<Dec, decimal32>::value, float, double>;
    std::uniform_real_distribution<comp_type> dist(0, 1e5);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::cbrt(val1)};
        auto ret_dec {static_cast<comp_type>(cbrt(d1))};

        if (!BOOST_TEST(boost::math::float_distance(ret_val, ret_dec) < 15))
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

template <typename Dec>
void test_two_val_hypot()
{
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto ret_val {std::hypot(val1, val2)};
        auto ret_dec {static_cast<float>(hypot(d1, d2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 1500))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    const auto big_val {dist(rng)};
    BOOST_TEST_EQ(hypot(Dec(big_val), Dec(big_val * 1e20F)), Dec(big_val * 1e20F));

    Dec inf {std::numeric_limits<Dec>::infinity() * static_cast<int>(dist(rng))};
    Dec nan {std::numeric_limits<Dec>::quiet_NaN() * static_cast<int>(dist(rng))};
    Dec zero {0 * static_cast<int>(dist(rng))};
    BOOST_TEST_EQ(hypot(zero, Dec(1)), Dec(1));
    BOOST_TEST_EQ(hypot(Dec(1), zero), Dec(1));
    BOOST_TEST(isinf(hypot(inf, Dec(1))));
    BOOST_TEST(isinf(hypot(Dec(1), inf)));
    BOOST_TEST(isnan(hypot(nan, Dec(1))));
    BOOST_TEST(isnan(hypot(Dec(1), nan)));
}

template <typename Dec1, typename Dec2>
void test_mixed_two_val_hypot()
{
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec2, decimal128>::value ? 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec1 d1 {val1};
        Dec2 d2 {val2};

        auto ret_val {std::hypot(val1, val2)};
        auto ret_dec {static_cast<float>(hypot(d1, d2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 1500))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

#if defined(__cpp_lib_hypot) && __cpp_lib_hypot >= 201603L

template <typename Dec>
void test_three_val_hypot()
{
    
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        const auto val3 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};
        Dec d3 {val3};

        auto ret_val {std::hypot(val1, val2, val3)};
        auto ret_dec {static_cast<float>(hypot(d1, d2, d3))};

        if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 1500))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nVal 3: " << val3
                      << "\nDec 3: " << d3
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    Dec inf {std::numeric_limits<Dec>::infinity() * static_cast<int>(dist(rng))};
    Dec nan {std::numeric_limits<Dec>::quiet_NaN() * static_cast<int>(dist(rng))};
    Dec zero {0 * static_cast<int>(dist(rng))};
    BOOST_TEST(isinf(hypot(inf, Dec(1), Dec(1))));
    BOOST_TEST(isinf(hypot(Dec(1), inf, Dec(1))));
    BOOST_TEST(isinf(hypot(Dec(1), Dec(1), inf)));
    BOOST_TEST(isnan(hypot(nan, Dec(1), Dec(1))));
    BOOST_TEST(isnan(hypot(Dec(1), nan, Dec(1))));
    BOOST_TEST(isnan(hypot(Dec(1), Dec(1), nan)));
}

#else

template <typename Dec>
void test_three_val_hypot()
{
    // Empty because we can't test against it yet
}

#endif

template <typename Dec>
void test_rint()
{
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {static_cast<float>(std::rint(val1))};
        auto ret_dec {static_cast<float>(rint(d1))};

        // Difference in default rounding mode
        // Float goes to even while decimal is to nearest from zero
        if (ret_val < val1 && ret_dec - 1 == ret_val)
        {
            continue;
        }

        if (std::fabs(val1) > 9'999'999.0F)
        {
            if(!BOOST_TEST(std::fabs(boost::math::float_distance(val1, ret_dec)) < 10))
            {
                // LCOV_EXCL_START
                std::cerr << std::setprecision(std::numeric_limits<Dec>::digits10)
                          << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
                // LCOV_EXCL_STOP
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist2(rng)};
        Dec d1 {val1};

        auto ret_val {static_cast<float>(std::rint(val1))};
        auto ret_dec {static_cast<float>(rint(d1))};

        // Difference in default rounding mode
        // Float goes to even while decimal is to nearest from zero
        float iptr {};
        const auto frac {std::modf(val1, &iptr)};
        if (std::abs(frac) <= 0.5F && std::abs(frac) >= 0.49F)
        {
            continue;
        }

        if (std::fabs(val1) > 9'999'999.0F)
        {
            if(!BOOST_TEST(std::fabs(boost::math::float_distance(val1, ret_dec)) < 10))
            {
                // LCOV_EXCL_START
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
                // LCOV_EXCL_STOP
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(rint(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)))));
    BOOST_TEST(isnan(rint(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST_EQ(rint(Dec(0) * Dec(dist(rng))), Dec(0));
    BOOST_TEST_EQ(rint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), Dec(0));
    BOOST_TEST_EQ(rint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), Dec(0, 0, true));
}

template <typename Dec>
void test_lrint()
{
    std::uniform_real_distribution<float> dist(-1e5F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::lrint(val1)};
        auto ret_dec {lrint(d1)};

        // Difference in rounding mode at 0.5
        if (ret_dec == ret_val + 1)
        {
            float iptr;
            const auto frac = modf(val1, &iptr);
            if (std::fabs(frac - 0.5F) < 0.01F)
            {
                continue;
            }
            else
            {
                // LCOV_EXCL_START
                std::cerr << "Frac: " << frac
                          << "\nDist: " << std::fabs(frac - 0.5F) / std::numeric_limits<float>::epsilon() << std::endl;
                // LCOV_EXCL_STOP
            }
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist2(rng)};
        Dec d1 {val1};

        auto ret_val {std::lrint(val1)};
        auto ret_dec {lrint(d1)};

        // Difference in significant figures
        float iptr {};
        const auto frac {std::modf(val1, &iptr)};
        if (std::abs(frac) <= 0.5F && std::abs(frac) >= 0.49F)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST_EQ(lrint(std::numeric_limits<Dec>::infinity() * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lrint(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lrint(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(lrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(lrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_llrint()
{
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::llrint(val1)};
        auto ret_dec {llrint(d1)};

        // Difference in significant figures
        if (ret_dec > 9'999'999 || ret_dec < -9'999'999)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist2(rng)};
        Dec d1 {val1};

        auto ret_val {std::llrint(val1)};
        auto ret_dec {llrint(d1)};

        // Difference in significant figures
        float iptr {};
        const auto frac {std::modf(val1, &iptr)};
        if (std::abs(frac) <= 0.5F && std::abs(frac) >= 0.49F)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST_EQ(llrint(std::numeric_limits<Dec>::infinity() * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llrint(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llrint(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(llrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(llrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_nearbyint()
{
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {static_cast<float>(std::nearbyint(val1))};
        auto ret_dec {static_cast<float>(nearbyint(d1))};

        // Difference in default rounding mode
        // Float goes to even while decimal is to nearest from zero
        if (ret_val < val1 && ret_dec - 1 == ret_val)
        {
            continue;
        }

        if (std::fabs(val1) > 9'999'999.0F)
        {
            if(!BOOST_TEST(std::fabs(boost::math::float_distance(val1, ret_dec)) < 10))
            {
                // LCOV_EXCL_START
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
                // LCOV_EXCL_STOP
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist2(rng)};
        Dec d1 {val1};

        auto ret_val {static_cast<float>(std::nearbyint(val1))};
        auto ret_dec {static_cast<float>(nearbyint(d1))};

        // Difference in default rounding mode
        // Float goes to even while decimal is to nearest from zero
        float iptr {};
        const auto frac {std::modf(val1, &iptr)};
        if (std::abs(frac) <= 0.5F && std::abs(frac) >= 0.49F)
        {
            continue;
        }

        if (std::fabs(val1) > 9'999'999.0F)
        {
            if(!BOOST_TEST(std::fabs(boost::math::float_distance(val1, ret_dec)) < 10))
            {
                // LCOV_EXCL_START
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
                // LCOV_EXCL_STOP
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(nearbyint(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)))));
    BOOST_TEST(isnan(nearbyint(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST_EQ(nearbyint(Dec(0) * Dec(dist(rng))), Dec(0));
    BOOST_TEST_EQ(nearbyint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), Dec(0));
    BOOST_TEST_EQ(nearbyint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), Dec(0, 0, true));
}

template <typename Dec>
void test_round()
{
    std::uniform_real_distribution<float> dist(-1e5F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {static_cast<float>(std::round(val1))};
        auto ret_dec {static_cast<float>(round(d1))};

        // Difference in default rounding mode
        // Float goes to even while decimal is to nearest from zero
        float iptr {};
        const auto frac {std::modf(val1, &iptr)};
        if (std::abs(frac) <= 0.5F && std::abs(frac) >= 0.49F)
        {
            continue;
        }

        if (std::fabs(val1) > 9'999'999.0F)
        {
            if(!BOOST_TEST(std::fabs(boost::math::float_distance(val1, ret_dec)) < 10))
            {
                // LCOV_EXCL_START
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
                // LCOV_EXCL_STOP
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(round(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)))));
    BOOST_TEST(isnan(round(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST_EQ(round(Dec(0) * Dec(dist(rng))), Dec(0));
    BOOST_TEST_EQ(round(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), Dec(0));
    BOOST_TEST_EQ(round(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), Dec(0, 0, false));
}

template <typename Dec>
void test_lround()
{
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::lround(val1)};
        auto ret_dec {lround(d1)};

        // Difference in significant figures
        if (ret_dec > 9'999'999 || ret_dec < -9'999'999)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist2(rng)};
        Dec d1 {val1};

        auto ret_val {std::lround(val1)};
        auto ret_dec {lround(d1)};

        // Difference in significant figures
        float iptr {};
        const auto frac {std::modf(val1, &iptr)};
        if (std::abs(frac) <= 0.5F && std::abs(frac) >= 0.49F)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST_EQ(lround(std::numeric_limits<Dec>::infinity() * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lround(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lround(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(lround(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(lround(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_llround()
{
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::llround(val1)};
        auto ret_dec {llround(d1)};

        // Difference in significant figures
        if (ret_dec > 9'999'999 || ret_dec < -9'999'999)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist2(rng)};
        Dec d1 {val1};

        auto ret_val {std::llround(val1)};
        auto ret_dec {llround(d1)};

        // Difference in significant figures
        float iptr {};
        const auto frac {std::modf(val1, &iptr)};
        if (std::abs(frac) <= 0.5F && std::abs(frac) >= 0.49F)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST_EQ(llround(std::numeric_limits<Dec>::infinity() * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llround(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llround(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(llround(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(llround(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_nextafter()
{
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto ret_val {std::nextafter(val1, val2)};
        auto ret_dec {static_cast<float>(nextafter(d1, d2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(boost::math::float_distance(ret_val, ret_dec) < 10))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nDist: " << boost::math::float_distance(ret_val, ret_dec);
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(nextafter(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(nextafter(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(nextafter(Dec(1), std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)))));
    BOOST_TEST(!isinf(nextafter(Dec(1), std::numeric_limits<Dec>::infinity() * Dec(dist(rng)))));
    BOOST_TEST_EQ(nextafter(Dec(1), Dec(1)), Dec(1));
    BOOST_TEST_EQ(nextafter(Dec(0), Dec(1)), std::numeric_limits<Dec>::epsilon());
    BOOST_TEST_EQ(nextafter(Dec(0), Dec(-1)), -std::numeric_limits<Dec>::epsilon());
}

template <typename Dec>
void test_nexttoward()
{
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};
    for (std::size_t n {}; n < max_iter; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto ret_val {std::nexttoward(val1, val2)};
        auto ret_dec {static_cast<float>(nexttoward(d1, val2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(boost::math::float_distance(ret_val, ret_dec) < 10))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nDist: " << boost::math::float_distance(ret_val, ret_dec);
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(nexttoward(std::numeric_limits<Dec>::infinity() * Dec(dist(rng)), 1)));
    BOOST_TEST(isnan(nexttoward(std::numeric_limits<Dec>::quiet_NaN() * Dec(dist(rng)), 1)));
    BOOST_TEST_EQ(nexttoward(Dec(1), 1), Dec(1));
    BOOST_TEST_EQ(nexttoward(Dec(0), 1), std::numeric_limits<Dec>::epsilon());
    BOOST_TEST_EQ(nexttoward(Dec(0), -1), -std::numeric_limits<Dec>::epsilon());
}

template <typename T>
void test_pow()
{
    std::uniform_int_distribution<int> dist(1, 1);

    const T two {2, 0};
    BOOST_TEST_EQ(pow(two, dist(rng)), two);
    BOOST_TEST_EQ(pow(two, 2 * dist(rng)), two * two);
    BOOST_TEST_EQ(pow(two, 3 * dist(rng)), two * two * two);
    BOOST_TEST_EQ(pow(two, 4 * dist(rng)), two * two * two * two);
}

template <typename T>
void test_exp2()
{
    std::uniform_int_distribution<int> dist(1, 1);

    const T two {2, 0};
    BOOST_TEST_EQ(exp2(two), two * two * dist(rng));
    BOOST_TEST_EQ(exp2(T(0 * dist(rng))), T(dist(rng)));
    BOOST_TEST(isnan(exp2(std::numeric_limits<T>::quiet_NaN())));
    BOOST_TEST(isinf(exp2(std::numeric_limits<T>::infinity())));
    BOOST_TEST_EQ(exp2(-std::numeric_limits<T>::infinity()), T(0 * dist(rng)));
}

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)
template <typename T>
void test_nan()
{
    BOOST_TEST(!isnan(nan<T>("1") & std::numeric_limits<T>::quiet_NaN()));
    BOOST_TEST(!isnan(nan<T>("2") & std::numeric_limits<T>::quiet_NaN()));
    BOOST_TEST(!isnan(nan<T>("-1") & std::numeric_limits<T>::quiet_NaN()));
}
#endif

int main()
{
    test_fmax<decimal32>();
    test_isgreater<decimal32>();
    test_isgreaterequal<decimal32>();
    test_fmin<decimal32>();
    test_isless<decimal32>();
    test_islessequal<decimal32>();
    test_islessgreater<decimal32>();
    test_isunordered<decimal32>();
    test_fmax<decimal64>();
    test_isgreater<decimal64>();
    test_isgreaterequal<decimal64>();
    test_fmin<decimal64>();
    test_isless<decimal64>();
    test_islessequal<decimal64>();
    test_islessgreater<decimal64>();
    test_isunordered<decimal64>();
    test_fmax<decimal128>();
    test_isgreater<decimal128>();
    test_isgreaterequal<decimal128>();
    test_fmin<decimal128>();
    test_isless<decimal128>();
    test_islessequal<decimal128>();
    test_islessgreater<decimal128>();
    test_isunordered<decimal128>();

    test_floor<decimal32>();
    test_ceil<decimal32>();
    test_trunc<decimal32>();
    test_floor<decimal64>();
    test_ceil<decimal64>();
    test_trunc<decimal64>();
    test_floor<decimal128>();
    test_ceil<decimal128>();
    test_trunc<decimal128>();

    test_frexp10<decimal32>();
    test_scalbn<decimal32>();
    test_scalbln<decimal32>();
    test_frexp10<decimal64>();
    test_scalbn<decimal64>();
    test_scalbln<decimal64>();

    test_div_fmod<decimal32>();
    test_div_fmod<decimal64>();

    test_copysign<decimal32>();
    test_copysign<decimal64>();

    test_fma<decimal32>();
    test_fma<decimal64>();
    test_fma<decimal128>();

    test_sin<decimal32>();
    test_cos<decimal32>();
    test_sin<decimal64>();
    test_cos<decimal64>();

    test_modf<decimal32>();
    test_modf<decimal64>();

    test_remainder<decimal32>();
    test_remquo<decimal32>();
    test_remainder<decimal64>();
    test_remquo<decimal64>();

    test_fdim<decimal32>();
    test_fdim<decimal64>();

    test_ilogb<decimal32>();
    test_ilogb<decimal64>();
    test_ilogb<decimal128>();

    test_logb<decimal32>();
    test_logb<decimal64>();
    test_logb<decimal128>();

    test_sqrt<decimal32>();
    test_sqrt<decimal64>();

    test_cbrt<decimal32>();
    test_cbrt<decimal64>();

    test_two_val_hypot<decimal32>();
    test_three_val_hypot<decimal32>();
    test_two_val_hypot<decimal64>();
    test_three_val_hypot<decimal64>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test_two_val_hypot<decimal128>();
    test_three_val_hypot<decimal128>();
    test_mixed_two_val_hypot<decimal64, decimal128>();
    #endif

    test_mixed_two_val_hypot<decimal32, decimal64>();

    test_rint<decimal32>();
    test_lrint<decimal32>();
    test_llrint<decimal32>();
    test_nearbyint<decimal32>();
    test_rint<decimal64>();
    test_lrint<decimal64>();
    test_llrint<decimal64>();
    test_nearbyint<decimal64>();

    test_round<decimal32>();
    test_lround<decimal32>();
    test_llround<decimal32>();
    test_round<decimal64>();
    test_lround<decimal64>();
    test_llround<decimal64>();

    test_nextafter<decimal32>();
    test_nexttoward<decimal32>();
    test_nextafter<decimal64>();
    test_nexttoward<decimal64>();

    test_pow<decimal32>();
    test_pow<decimal64>();

    test_exp2<decimal32>();
    test_exp2<decimal64>();

    #if !defined(BOOST_DECIMAL_DISABLE_CLIB)
    test_nan<decimal32>();
    test_nan<decimal64>();
    test_nan<decimal128>();
    #endif

    return boost::report_errors();
}
