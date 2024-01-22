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
    #ifdef BOOST_DECIMAL_GENERATE_CONSTANT_SIGS
    print_value("1.5699342435209476025651717041741222e-19"_DL, "a0");
    print_value("-8.8132732956573375066199565461908719e-18"_DL, "a1");
    print_value("1.3863524195221541686408628558203953e-18"_DL, "a2");
    print_value("2.8092138674699095038603966669197672e-15"_DL, "a3");
    print_value("2.6602926311836597886374265715483009e-18"_DL, "a4");
    print_value("-7.647187644614729441998912554111624e-13"_DL, "a5");
    print_value("1.6631026334876884745954474452210885e-18"_DL, "a6");
    print_value("1.6059043746245914318033425521094091e-10"_DL, "a7");
    print_value("3.8855818741866909397363716405229141e-19"_DL, "a8");
    print_value("-2.5052108385573133011847226381434009e-08"_DL, "a9");
    print_value("3.4914886265734843190231514748823632e-20"_DL, "a10");
    print_value("2.7557319223985818369477273553859578e-06"_DL, "a11");
    print_value("1.1508316403034518496534319639927136e-21"_DL, "a12");
    print_value("-0.00019841269841269841283663285131581154"_DL, "a13");
    print_value("1.2185770421264807218078195121820301e-23"_DL, "a14");
    print_value("0.0083333333333333333333325747157858247"_DL, "a15");
    print_value("3.151985000765337776035625655498396e-26"_DL, "a16");
    print_value("-0.166666666666666666666666667468363"_DL, "a17");
    print_value("1.0798913896208528750785230421502658e-29"_DL, "a18");
    print_value("0.9999999999999999999999999999994222"_DL, "a19");
    print_value("5.142496035903513218983541015724909e-35"_DL, "a20");

    throw;
    #endif

    test_sin<decimal32>();
    test_cos<decimal32>();
    test_sin<decimal64>();
    test_cos<decimal64>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    //test_sin<decimal128>();
    //test_cos<decimal128>();
    #endif

    return boost::report_errors();
}
