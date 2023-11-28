// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;
using namespace boost::decimal::numbers;

template <typename Dec>
void test_constants()
{
    BOOST_TEST_EQ(Dec(2.718281828459045235), e_v<Dec>);
    BOOST_TEST_EQ(Dec(1.442695040888963407), log2e_v<Dec>);
    BOOST_TEST_EQ(Dec(0.4342944819032518277), log10e_v<Dec>);
    BOOST_TEST_EQ(Dec(3.141592653589793238), pi_v<Dec>);
    BOOST_TEST_EQ(Dec(0.3183098861837906715), inv_pi_v<Dec>);
    BOOST_TEST_EQ(Dec(0.5641895835477562869), inv_sqrtpi_v<Dec>);
    BOOST_TEST_EQ(Dec(0.6931471805599453094), ln2_v<Dec>);
    BOOST_TEST_EQ(Dec(2.302585092994045684), ln10_v<Dec>);
    BOOST_TEST_EQ(Dec(1.414213562373095049), sqrt2_v<Dec>);
    BOOST_TEST_EQ(Dec(1.732050807568877294), sqrt3_v<Dec>);
    BOOST_TEST(abs(Dec(0.707106781186547524) - inv_sqrt2_v<Dec>) <= std::numeric_limits<Dec>::epsilon());
    BOOST_TEST(abs(Dec(0.5773502691896257645) - inv_sqrt3_v<Dec>) <= std::numeric_limits<Dec>::epsilon());
    BOOST_TEST_EQ(Dec(0.5772156649015328606), egamma_v<Dec>);
    BOOST_TEST_EQ(Dec(1.618033988749894848), phi_v<Dec>);
}

template <>
void test_constants<decimal128>()
{
    BOOST_TEST_EQ("2.718281828459045235360287471352662"_DL, e_v<decimal128>);
    BOOST_TEST_EQ("1.4426950408889634073599246810018921"_DL, log2e_v<decimal128>);
    BOOST_TEST_EQ("0.43429448190325182765112891891660508"_DL, log10e_v<decimal128>);
    BOOST_TEST_EQ("3.1415926535897932384626433832795029"_DL, pi_v<decimal128>);
    BOOST_TEST_EQ("0.31830988618379067153776752674502872"_DL, inv_pi_v<decimal128>);
    BOOST_TEST_EQ("0.56418958354775628694807945156077259"_DL, inv_sqrtpi_v<decimal128>);
    BOOST_TEST_EQ("0.69314718055994530941723212145817657"_DL, ln2_v<decimal128>);
    BOOST_TEST_EQ("2.3025850929940456840179914546843642"_DL, ln10_v<decimal128>);
    BOOST_TEST_EQ("1.4142135623730950488016887242096981"_DL, sqrt2_v<decimal128>);
    BOOST_TEST_EQ("1.7320508075688772935274463415058724"_DL, sqrt3_v<decimal128>);
    BOOST_TEST_EQ("0.70710678118654752440084436210484904"_DL, inv_sqrt2_v<decimal128>);
    BOOST_TEST_EQ("0.57735026918962576450914878050195746"_DL, inv_sqrt3_v<decimal128>);
    BOOST_TEST_EQ("0.57721566490153286060651209008240243"_DL, egamma_v<decimal128>);
    BOOST_TEST_EQ("1.6180339887498948482045868343656381"_DL, phi_v<decimal128>);

    BOOST_TEST_EQ(static_cast<decimal64>(e_v<decimal128>), e_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(log2e_v<decimal128>), log2e_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(log10e_v<decimal128>), log10e_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(pi_v<decimal128>), pi_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(inv_pi_v<decimal128>), inv_pi_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(inv_sqrtpi_v<decimal128>), inv_sqrtpi_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(ln2_v<decimal128>), ln2_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(ln10_v<decimal128>), ln10_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(sqrt2_v<decimal128>), sqrt2_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(sqrt3_v<decimal128>), sqrt3_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(inv_sqrt2_v<decimal128>), inv_sqrt2_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(inv_sqrt3_v<decimal128>), inv_sqrt3_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(egamma_v<decimal128>), egamma_v<decimal64>);
    BOOST_TEST_EQ(static_cast<decimal64>(phi_v<decimal128>), phi_v<decimal64>);
}

void test_defaults()
{
    BOOST_TEST_EQ(decimal64(2.718281828459045235), e);
    BOOST_TEST_EQ(decimal64(1.442695040888963407), log2e);
    BOOST_TEST_EQ(decimal64(0.4342944819032518277), log10e);
    BOOST_TEST_EQ(decimal64(3.141592653589793238), pi);
    BOOST_TEST_EQ(decimal64(0.3183098861837906715), inv_pi);
    BOOST_TEST_EQ(decimal64(0.5641895835477562869), inv_sqrtpi);
    BOOST_TEST_EQ(decimal64(0.6931471805599453094), ln2);
    BOOST_TEST_EQ(decimal64(2.302585092994045684), ln10);
    BOOST_TEST_EQ(decimal64(1.414213562373095049), sqrt2);
    BOOST_TEST_EQ(decimal64(1.732050807568877294), sqrt3);
    BOOST_TEST(abs(decimal64(0.707106781186547524) - inv_sqrt2_v<decimal64>) <= std::numeric_limits<decimal64>::epsilon());
    BOOST_TEST(abs(decimal64(0.5773502691896257645) - inv_sqrt3_v<decimal64>) <= std::numeric_limits<decimal64>::epsilon());
    BOOST_TEST_EQ(decimal64(0.5772156649015328606), egamma);
    BOOST_TEST_EQ(decimal64(1.618033988749894848), phi);
}

int main()
{
    test_constants<decimal32>();
    test_constants<decimal64>();
    test_constants<decimal128>();
    test_defaults();

    return boost::report_errors();
}
