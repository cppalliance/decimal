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

static constexpr auto N {1024};

using namespace boost::decimal;

template <typename Dec>
void test_fmax()
{
    std::mt19937_64 rng;
    std::uniform_int_distribution<int> dist(0, 10);

    BOOST_TEST_EQ(fmax(Dec(1), BOOST_DECIMAL_DEC_NAN * Dec(dist(rng))), Dec(1));
    BOOST_TEST_EQ(fmax(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), Dec(1)), Dec(1));
    BOOST_TEST(isnan(fmax(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST_EQ(fmax(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)), -BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng))), BOOST_DECIMAL_DEC_INFINITY);

    BOOST_TEST_EQ(fmax(Dec(1), Dec(0)), Dec(1));
    BOOST_TEST_EQ(fmax(Dec(-2), Dec(1)), Dec(1));
}

template <typename Dec>
void test_isgreater()
{
    BOOST_TEST_EQ(isgreater(Dec(1), BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(isgreater(BOOST_DECIMAL_DEC_NAN, Dec(1)), false);
    BOOST_TEST_EQ(isgreater(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(isgreater(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), true);

    BOOST_TEST_EQ(isgreater(Dec(1), Dec(0)), true);
    BOOST_TEST_EQ(isgreater(Dec(-2), Dec(1)), false);
    BOOST_TEST_EQ(isgreater(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_isgreaterequal()
{
    BOOST_TEST_EQ(isgreaterequal(Dec(1), BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(isgreaterequal(BOOST_DECIMAL_DEC_NAN, Dec(1)), false);
    BOOST_TEST_EQ(isgreaterequal(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(isgreaterequal(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), true);

    BOOST_TEST_EQ(isgreaterequal(Dec(1), Dec(0)), true);
    BOOST_TEST_EQ(isgreaterequal(Dec(-2), Dec(1)), false);
    BOOST_TEST_EQ(isgreaterequal(Dec(1), Dec(1)), true);
}

template <typename Dec>
void test_fmin()
{
    BOOST_TEST_EQ(fmin(Dec(1), BOOST_DECIMAL_DEC_NAN), Dec(1));
    BOOST_TEST_EQ(fmin(BOOST_DECIMAL_DEC_NAN, Dec(1)), Dec(1));
    BOOST_TEST(isnan(fmin(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN)));
    BOOST_TEST_EQ(fmin(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), -BOOST_DECIMAL_DEC_INFINITY);

    BOOST_TEST_EQ(fmin(Dec(1), Dec(0)), Dec(0));
    BOOST_TEST_EQ(fmin(Dec(-2), Dec(1)), Dec(-2));
}

template <typename Dec>
void test_isless()
{
    BOOST_TEST_EQ(isless(Dec(1), BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(isless(BOOST_DECIMAL_DEC_NAN, Dec(1)), false);
    BOOST_TEST_EQ(isless(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(isless(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), false);

    BOOST_TEST_EQ(isless(Dec(1), Dec(0)), false);
    BOOST_TEST_EQ(isless(Dec(-2), Dec(1)), true);
    BOOST_TEST_EQ(isless(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_islessequal()
{
    BOOST_TEST_EQ(islessequal(Dec(1), BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(islessequal(BOOST_DECIMAL_DEC_NAN, Dec(1)), false);
    BOOST_TEST_EQ(islessequal(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(islessequal(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), false);

    BOOST_TEST_EQ(islessequal(Dec(1), Dec(0)), false);
    BOOST_TEST_EQ(islessequal(Dec(-2), Dec(1)), true);
    BOOST_TEST_EQ(islessequal(Dec(1), Dec(1)), true);
}

template <typename Dec>
void test_islessgreater()
{
    BOOST_TEST_EQ(islessgreater(Dec(1), BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(islessgreater(BOOST_DECIMAL_DEC_NAN, Dec(1)), false);
    BOOST_TEST_EQ(islessgreater(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN), false);
    BOOST_TEST_EQ(islessgreater(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), true);

    BOOST_TEST_EQ(islessgreater(Dec(1), Dec(0)), true);
    BOOST_TEST_EQ(islessgreater(Dec(-2), Dec(1)), true);
    BOOST_TEST_EQ(islessgreater(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_isunordered()
{
    BOOST_TEST_EQ(isunordered(Dec(1), BOOST_DECIMAL_DEC_NAN), true);
    BOOST_TEST_EQ(isunordered(BOOST_DECIMAL_DEC_NAN, Dec(1)), true);
    BOOST_TEST_EQ(isunordered(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN), true);
    BOOST_TEST_EQ(isunordered(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), false);

    BOOST_TEST_EQ(isunordered(Dec(1), Dec(0)), false);
    BOOST_TEST_EQ(isunordered(Dec(-2), Dec(1)), false);
    BOOST_TEST_EQ(isunordered(Dec(1), Dec(1)), false);
}

template <typename Dec>
void test_floor()
{
    std::mt19937_64 rng;
    std::uniform_int_distribution<int> dist(1, 1);

    BOOST_TEST(isnan(floor(BOOST_DECIMAL_DEC_NAN * dist(rng))));
    BOOST_TEST(isnan(floor(-BOOST_DECIMAL_DEC_NAN * dist(rng))));
    BOOST_TEST(isinf(floor(BOOST_DECIMAL_DEC_INFINITY * dist(rng))));
    BOOST_TEST(isinf(floor(-BOOST_DECIMAL_DEC_INFINITY * dist(rng))));
    BOOST_TEST_EQ(floor(Dec(0, 0) * dist(rng)), Dec(0, 0));
    BOOST_TEST_EQ(floor(Dec(-0, 0) * dist(rng)), Dec(-0, 0));

    BOOST_TEST_EQ(floor(Dec(27, -1)), Dec(2, 0) * dist(rng));
    BOOST_TEST_EQ(floor(Dec(-27, -1)), Dec(-3, 0) * dist(rng));
    BOOST_TEST_EQ(floor(Dec(27777, -4)), Dec(2, 0) * dist(rng));
    BOOST_TEST_EQ(floor(Dec(-27777, -4)), Dec(-3, 0) * dist(rng));

    // Bigger numbers
    BOOST_TEST_EQ(floor(Dec(27777, -2)), Dec(277, 0) * dist(rng));
    BOOST_TEST_EQ(floor(Dec(-27777, -2)), Dec(-277, 0) * dist(rng));
    BOOST_TEST_EQ(floor(Dec(27777, -1)), Dec(2777, 0) * dist(rng));

    // Near zero
    BOOST_TEST_EQ(floor(Dec(3, -1)), Dec(0, 0) * dist(rng));
    BOOST_TEST_EQ(floor(Dec(-3, -1)), Dec(-1, 0) * dist(rng));
}

template <typename Dec>
void test_ceil()
{
    std::mt19937_64 rng;
    std::uniform_int_distribution<int> dist(0, 10);

    BOOST_TEST(isnan(ceil(BOOST_DECIMAL_DEC_NAN * dist(rng))));
    BOOST_TEST(isnan(ceil(-BOOST_DECIMAL_DEC_NAN * dist(rng))));
    BOOST_TEST(isinf(ceil(BOOST_DECIMAL_DEC_INFINITY * dist(rng))));
    BOOST_TEST(isinf(ceil(-BOOST_DECIMAL_DEC_INFINITY * dist(rng))));
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
    BOOST_TEST(isnan(trunc(BOOST_DECIMAL_DEC_NAN)));
    BOOST_TEST(isnan(trunc(-BOOST_DECIMAL_DEC_NAN)));
    BOOST_TEST(isinf(trunc(BOOST_DECIMAL_DEC_INFINITY)));
    BOOST_TEST(isinf(trunc(-BOOST_DECIMAL_DEC_INFINITY)));
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
    BOOST_TEST_EQ(frexp10(BOOST_DECIMAL_DEC_NAN, &expval), -1);
    BOOST_TEST_EQ(expval, 0);

    expval = -1;
    BOOST_TEST_EQ(frexp10(BOOST_DECIMAL_DEC_INFINITY, &expval), -1);
    BOOST_TEST_EQ(expval, 0);

    BOOST_TEST_EQ(frexp10(Dec(10, 0), &expval), 1'000'000);
    BOOST_TEST_EQ(expval, -5);

    BOOST_TEST_EQ(frexp10(Dec(1'000'000, 5), &expval), 1'000'000);
    BOOST_TEST_EQ(expval, 5);

    BOOST_TEST_EQ(frexp10(Dec(-1'000'000, 5), &expval), 1'000'000);
    BOOST_TEST_EQ(expval, 5);
}

template <typename Dec>
void test_scalbn()
{
    BOOST_TEST(isinf(scalbn(BOOST_DECIMAL_DEC_INFINITY, 1)));
    BOOST_TEST(isnan(scalbn(BOOST_DECIMAL_DEC_NAN, 1)));
    BOOST_TEST_EQ(scalbn(Dec(0, 0), 1), Dec(0, 0));

    decimal32 one {1, 0};
    decimal32 ten {1, 1};
    decimal32 hundred {1, 2};

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
    BOOST_TEST(isinf(scalbln(BOOST_DECIMAL_DEC_INFINITY, 1)));
    BOOST_TEST(isnan(scalbln(BOOST_DECIMAL_DEC_NAN, 1)));
    BOOST_TEST_EQ(scalbln(Dec(0, 0), 1), Dec(0, 0));

    decimal32 one {1, 0};
    decimal32 ten {1, 1};
    decimal32 hundred {1, 2};

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
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(0.0F, 1e30F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};

        auto float_div {val1 / val2};
        auto decimal_div {static_cast<float>(d1 / d2)};
        auto float_fmod {std::fmod(val1, val2)};
        auto decimal_fmod {static_cast<float>(fmod(d1, d2))};

        // Decimal types are all higher precision than float
        if (!(BOOST_TEST(std::fabs(boost::math::float_distance(float_fmod, decimal_fmod)) < 1e7) &&
              BOOST_TEST(std::fabs(boost::math::float_distance(float_div, decimal_div)) < 20)))
        {
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
        }
    }
}

template <typename Dec>
void test_copysign()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(0.0F, 1e30F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};

        auto ret_val {std::copysign(val1, val2)};
        auto ret_dec {static_cast<float>(copysign(d1, d2))};

        if (!BOOST_TEST(std::fabs(boost::math::float_distance(ret_val, ret_dec)) < 20))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }
}

template <typename Dec>
void test_fma()
{
    BOOST_TEST_EQ(fma(decimal32(1, -1), decimal32(1, 1), decimal32(1, 0, true)), decimal32(0, 0));

    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e10F, 1e10F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        const auto val3 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};
        decimal32 d3 {val3};

        auto fma_val {fma(d1, d2, d3)};
        auto naive_val {(d1 * d2) + d3};

        if (!BOOST_TEST(fabs(fma_val - naive_val) < std::numeric_limits<Dec>::epsilon()))
        {
            std::cerr << "Dec 1: " << d1
                      << "\nDec 2: " << d2
                      << "\nDec 3: " << d3
                      << "\nfma val: " << fma_val
                      << "\nNaive val: " << naive_val << std::endl;
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
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-3.14F * 2, 3.14F * 2);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        decimal32 d1 {val1};

        auto ret_val {std::sin(val1)};
        auto ret_dec {static_cast<float>(sin(d1))};

        if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 30*std::numeric_limits<float>::epsilon()))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    BOOST_TEST(isinf(sin(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)))));
    BOOST_TEST(isnan(sin(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST_EQ(sin(Dec(0) * Dec(dist(rng))), Dec(0));
}

template <typename Dec>
void test_cos()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-3.14F * 2, 3.14F * 2);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        decimal32 d1 {val1};

        auto ret_val {std::cos(val1)};
        auto ret_dec {static_cast<float>(cos(d1))};

        if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 25*std::numeric_limits<float>::epsilon()))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    BOOST_TEST(isinf(cos(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)))));
    BOOST_TEST(isnan(cos(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST_EQ(cos(Dec(0) * Dec(dist(rng))), Dec(1));
}

template <typename Dec>
void test_modf()
{
    Dec ptr {};
    BOOST_TEST_EQ(modf(decimal32(123.45), &ptr), Dec(0.45));
    BOOST_TEST_EQ(ptr, Dec(123));

    BOOST_TEST_EQ(modf(BOOST_DECIMAL_DEC_INFINITY, &ptr), Dec(0));
    BOOST_TEST_EQ(ptr, BOOST_DECIMAL_DEC_INFINITY);

    BOOST_TEST(modf(BOOST_DECIMAL_DEC_NAN, &ptr) != BOOST_DECIMAL_DEC_NAN);
    BOOST_TEST(ptr != BOOST_DECIMAL_DEC_NAN);
}

template <typename Dec>
void test_remainder()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(1e2F, 1e3F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};

        auto ret_val {std::remainder(val1, val2)};
        auto ret_dec {static_cast<float>(remainder(d1, d2))};

        if (!BOOST_TEST(std::fabs(boost::math::float_distance(ret_val, ret_dec)) < 2000))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nFloat dist: " << boost::math::float_distance(ret_val, ret_dec) << std::endl;
        }
    }

    BOOST_TEST(isnan(remainder(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(remainder(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(remainder(Dec(1), BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST(isnan(remainder(Dec(1), Dec(0))));
}

template <typename Dec>
void test_remquo()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(1e2F, 1e3F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};
        int flt_int {};
        int dec_int {};

        auto ret_val {std::remquo(val1, val2, &flt_int)};
        auto ret_dec {static_cast<float>(remquo(d1, d2, &dec_int))};

        if (!(BOOST_TEST(std::fabs(boost::math::float_distance(ret_val, ret_dec)) < 2000) &&
             (BOOST_TEST(flt_int == dec_int))))
        {
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
        }
    }

    int quo {};
    BOOST_TEST(isnan(remquo(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)), Dec(1), &quo)));
    BOOST_TEST(isnan(remquo(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), Dec(1), &quo)));
    BOOST_TEST(isnan(remquo(Dec(1), BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), &quo)));
    BOOST_TEST(isnan(remquo(Dec(1), Dec(0), &quo)));
}

template <typename Dec>
void test_fdim()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};

        auto ret_val {std::fdim(val1, val2)};
        auto ret_dec {static_cast<float>(fdim(d1, d2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 1))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    BOOST_TEST(isinf(fdim(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(fdim(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(fdim(Dec(1), BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST_EQ(fdim(Dec(1), Dec(1)), Dec(0));
}

template <typename Dec>
void test_ilogb()
{
    BOOST_TEST_EQ(ilogb(Dec(1,0)), 101);
    BOOST_TEST_EQ(ilogb(Dec(10, 0)), 102);
    BOOST_TEST_EQ(ilogb(Dec(0)), FP_ILOGB0);
    BOOST_TEST_EQ(ilogb(BOOST_DECIMAL_DEC_INFINITY), INT_MAX);
    BOOST_TEST_EQ(ilogb(BOOST_DECIMAL_DEC_NAN), FP_ILOGBNAN);
}

template <typename Dec>
void test_sqrt()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(0.0F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        decimal32 d1 {val1};

        auto ret_val {std::sqrt(val1)};
        auto ret_dec {static_cast<float>(sqrt(d1))};

        if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 1500))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    decimal32 inf {BOOST_DECIMAL_DEC_INFINITY * static_cast<int>(dist(rng))};
    decimal32 nan {BOOST_DECIMAL_DEC_NAN * static_cast<int>(dist(rng))};
    decimal32 zero {0 * static_cast<int>(dist(rng))};
    decimal32 neg_num {-static_cast<int>(dist(rng))};
    BOOST_TEST(isinf(sqrt(inf)));
    BOOST_TEST(isnan(sqrt(-inf)));
    BOOST_TEST(isnan(sqrt(nan)));
    BOOST_TEST(isnan(sqrt(-nan)));
    BOOST_TEST_EQ(sqrt(zero), zero);
    BOOST_TEST(isnan(sqrt(neg_num)));
}

template <typename Dec>
void test_two_val_hypot()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};

        auto ret_val {std::hypot(val1, val2)};
        auto ret_dec {static_cast<float>(hypot(d1, d2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 1500))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    const auto big_val {dist(rng)};
    BOOST_TEST_EQ(hypot(Dec(big_val), Dec(big_val * 1e20F)), Dec(big_val * 1e20F));

    decimal32 inf {BOOST_DECIMAL_DEC_INFINITY * static_cast<int>(dist(rng))};
    decimal32 nan {BOOST_DECIMAL_DEC_NAN * static_cast<int>(dist(rng))};
    decimal32 zero {0 * static_cast<int>(dist(rng))};
    BOOST_TEST_EQ(hypot(zero, Dec(1)), Dec(1));
    BOOST_TEST_EQ(hypot(Dec(1), zero), Dec(1));
    BOOST_TEST(isinf(hypot(inf, Dec(1))));
    BOOST_TEST(isinf(hypot(Dec(1), inf)));
    BOOST_TEST(isnan(hypot(nan, Dec(1))));
    BOOST_TEST(isnan(hypot(Dec(1), nan)));
}

#if defined(__cpp_lib_hypot) && __cpp_lib_hypot >= 201603L

template <typename Dec>
void test_three_val_hypot()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nVal 3: " << val3
                      << "\nDec 3: " << d3
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    decimal32 inf {BOOST_DECIMAL_DEC_INFINITY * static_cast<int>(dist(rng))};
    decimal32 nan {BOOST_DECIMAL_DEC_NAN * static_cast<int>(dist(rng))};
    decimal32 zero {0 * static_cast<int>(dist(rng))};
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
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    for (std::size_t n {}; n < N; ++n)
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
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    BOOST_TEST(isinf(rint(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)))));
    BOOST_TEST(isnan(rint(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST_EQ(rint(Dec(0) * Dec(dist(rng))), Dec(0));
    BOOST_TEST_EQ(rint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), Dec(0));
    BOOST_TEST_EQ(rint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), Dec(0, 0, true));
}

template <typename Dec>
void test_lrint()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        Dec d1 {val1};

        auto ret_val {std::lrint(val1)};
        auto ret_dec {lrint(d1)};

        // Difference in significant figures
        if (ret_dec > 9'999'999 || ret_dec < -9'999'999)
        {
            continue;
        }

        if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    BOOST_TEST_EQ(lrint(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lrint(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lrint(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(lrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(lrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_llrint()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    BOOST_TEST_EQ(llrint(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llrint(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llrint(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(llrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(llrint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_nearbyint()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    for (std::size_t n {}; n < N; ++n)
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
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    BOOST_TEST(isinf(nearbyint(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)))));
    BOOST_TEST(isnan(nearbyint(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST_EQ(nearbyint(Dec(0) * Dec(dist(rng))), Dec(0));
    BOOST_TEST_EQ(nearbyint(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), Dec(0));
    BOOST_TEST_EQ(nearbyint(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), Dec(0, 0, true));
}

template <typename Dec>
void test_round()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e5F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
                std::cerr << "Val 1: " << val1
                          << "\nDec 1: " << d1
                          << "\nRet val: " << ret_val
                          << "\nRet dec: " << ret_dec
                          << "\nDist: " << boost::math::float_distance(val1, ret_dec) << std::endl;
            }
        }
        else if (!BOOST_TEST_EQ(ret_val, ret_dec))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
        }
    }

    BOOST_TEST(isinf(round(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)))));
    BOOST_TEST(isnan(round(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST_EQ(round(Dec(0) * Dec(dist(rng))), Dec(0));
    BOOST_TEST_EQ(round(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), Dec(0));
    BOOST_TEST_EQ(round(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), Dec(0, 0, false));
}

template <typename Dec>
void test_lround()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    BOOST_TEST_EQ(lround(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lround(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng))), LONG_MIN);
    BOOST_TEST_EQ(lround(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(lround(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(lround(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_llround()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(-1e20F, 1e20F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    std::uniform_real_distribution<float> dist2(-1e5F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
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
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec << std::endl;
        }
    }

    BOOST_TEST_EQ(llround(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llround(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng))), LLONG_MIN);
    BOOST_TEST_EQ(llround(Dec(0) * Dec(dist(rng))), 0);
    BOOST_TEST_EQ(llround(Dec(0) * Dec(dist(rng)) + Dec(1, -20)), 0);
    BOOST_TEST_EQ(llround(Dec(0) * Dec(dist(rng)) + Dec(1, -20, true)), 0);
}

template <typename Dec>
void test_nextafter()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};

        auto ret_val {std::nextafter(val1, val2)};
        auto ret_dec {static_cast<float>(nextafter(d1, d2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(boost::math::float_distance(ret_val, ret_dec) < 10))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nDist: " << boost::math::float_distance(ret_val, ret_dec);
        }
    }

    BOOST_TEST(isinf(nextafter(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(nextafter(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), Dec(1))));
    BOOST_TEST(isnan(nextafter(Dec(1), BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)))));
    BOOST_TEST(!isinf(nextafter(Dec(1), BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)))));
    BOOST_TEST_EQ(nextafter(Dec(1), Dec(1)), Dec(1));
    BOOST_TEST_EQ(nextafter(Dec(0), Dec(1)), std::numeric_limits<Dec>::epsilon());
    BOOST_TEST_EQ(nextafter(Dec(0), Dec(-1)), -std::numeric_limits<Dec>::epsilon());
}

template <typename Dec>
void test_nexttoward()
{
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<float> dist(1.0F, 1e5F);

    for (std::size_t n {}; n < N; ++n)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};
        decimal32 d1 {val1};
        decimal32 d2 {val2};

        auto ret_val {std::nexttoward(val1, val2)};
        auto ret_dec {static_cast<float>(nexttoward(d1, val2))};

        if (ret_val == 0 || ret_dec == 0)
        {
            BOOST_TEST_EQ(ret_val, ret_dec);
        }
        else if (!BOOST_TEST(boost::math::float_distance(ret_val, ret_dec) < 10))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << d1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << d2
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nDist: " << boost::math::float_distance(ret_val, ret_dec);
        }
    }

    BOOST_TEST(isinf(nexttoward(BOOST_DECIMAL_DEC_INFINITY * Dec(dist(rng)), 1)));
    BOOST_TEST(isnan(nexttoward(BOOST_DECIMAL_DEC_NAN * Dec(dist(rng)), 1)));
    BOOST_TEST_EQ(nexttoward(Dec(1), 1), Dec(1));
    BOOST_TEST_EQ(nexttoward(Dec(0), 1), std::numeric_limits<Dec>::epsilon());
    BOOST_TEST_EQ(nexttoward(Dec(0), -1), -std::numeric_limits<Dec>::epsilon());
}

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

    test_floor<decimal32>();
    test_ceil<decimal32>();
    test_trunc<decimal32>();

    test_frexp10<decimal32>();
    test_scalbn<decimal32>();
    test_scalbln<decimal32>();

    test_div_fmod<decimal32>();

    test_copysign<decimal32>();

    test_fma<decimal32>();

    test_sin<decimal32>();
    test_cos<decimal32>();

    test_modf<decimal32>();

    test_remainder<decimal32>();
    test_remquo<decimal32>();

    test_fdim<decimal32>();

    test_ilogb<decimal32>();

    test_sqrt<decimal32>();

    test_two_val_hypot<decimal32>();
    test_three_val_hypot<decimal32>();

    test_rint<decimal32>();
    test_lrint<decimal32>();
    test_llrint<decimal32>();
    test_nearbyint<decimal32>();

    test_round<decimal32>();
    test_lround<decimal32>();
    test_llround<decimal32>();

    test_nextafter<decimal32>();
    test_nexttoward<decimal32>();

    return boost::report_errors();
}
