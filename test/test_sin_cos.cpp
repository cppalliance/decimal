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
    print_value("436.021684388252698008009005087997361"_DL, "a0");
    print_value("-4039.21638981374780301605091138334041"_DL, "a1");
    print_value("18097.000997294144904446653101142636"_DL, "a2");
    print_value("-52177.282596450846709087608773379376"_DL, "a3");
    print_value("108708.48432550268314425281408153419"_DL, "a4");
    print_value("-174262.5954622716602957172670677698"_DL, "a5");
    print_value("223493.57422415636689935147124438032"_DL, "a6");
    print_value("-235454.321064437777710838740822821291"_DL, "a7");
    print_value("207630.98777755980752169002184822046"_DL, "a8");
    print_value("-155396.69461689412343100720698901834"_DL, "a9");
    print_value("99746.039943987449311614022410223385"_DL, "a10");
    print_value("-55350.309707359479385689369358768157"_DL, "a11");
    print_value("26715.867014556930852695472313352703"_DL, "a12");
    print_value("-11268.338244572170249967527464615569"_DL, "a13");
    print_value("4167.6576373496175687445580059590862"_DL, "a14");
    print_value("-1354.9661562313129205074831860545212"_DL, "a15");
    print_value("387.85110009446107984667231846782873"_DL, "a16");
    print_value("-97.817489091457389284370953518072941"_DL, "a17");
    print_value("21.743307363075844418345038236692007"_DL, "a18");
    print_value("-4.2480133538896834744681916927887348"_DL, "a19");
    print_value("0.73292313819882720927775694986695822"_DL, "a20");
    print_value("-0.1010376444122805424367275705575875"_DL, "a21");
    print_value("0.014666852097615399958677037388739653"_DL, "a22");
    print_value("0.0098574983213099696795218405193307645"_DL, "a23");
    print_value("0.00017008786793851718237248325423866098"_DL, "a24");
    print_value("0.013950081361102626885069793269505888"_DL, "a25");
    print_value("1.1009122133007665306092568908271278e-06"_DL, "a26");
    print_value("0.017352694401278245203946951656646601"_DL, "a27");
    print_value("3.7646439494486735639855121728483381e-09"_DL, "a28");
    print_value("0.022372158921646811423908478031651474"_DL, "a29");
    print_value("6.2818862516685652814845387053772962e-12"_DL, "a30");
    print_value("0.030381944444255037245064480562068303"_DL, "a31");
    print_value("4.5487997852735016804398755617727032e-15"_DL, "a32");
    print_value("0.044642857142857057989046867608414157"_DL, "a33");
    print_value("1.190951954325925523311588515718388e-18"_DL, "a34");
    print_value("0.074999999999999999987961873655488348"_DL, "a35");
    print_value("8.2501737347587131207284128263397492e-23"_DL, "a36");
    print_value("0.16666666666666666666666631637239263"_DL, "a37");
    print_value("7.9432557432242856979871267315136865e-28"_DL, "a38");
    print_value("0.99999999999999999999999999999928086"_DL, "a39");
    print_value("1.0845024738180057189197205194839397e-34"_DL, "a40");

    throw;
    #endif

    test_sin<decimal32>();
    test_sin<decimal64>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test_sin<decimal128>();
    #endif

    return boost::report_errors();
}
