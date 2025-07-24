// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal/decimal32_t.hpp>
#include <boost/decimal/decimal64_t.hpp>
#include <boost/decimal/decimal128_t.hpp>
#include <boost/decimal/decimal_fast32_t.hpp>
#include <boost/decimal/decimal_fast64_t.hpp>
#include <boost/decimal/decimal_fast128_t.hpp>
#include <boost/decimal/literals.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/iostream.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;
using namespace boost::decimal::numbers;
using namespace boost::decimal::literals;

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

template <typename T>
void print_value(T value, const char* str)
{
    int ptr;
    const auto sig_val = frexp10(value, &ptr);
    std::cerr << std::setprecision(std::numeric_limits<T>::digits10) << str << ": " << value
              << "\nSig: " << sig_val.high << " " << sig_val.low
              << "\nExp: " << ptr << "\n" << std::endl;
}

template <>
void test_constants<decimal128_t>()
{
    BOOST_TEST_EQ("2.718281828459045235360287471352662"_DL, e_v<decimal128_t>);
    BOOST_TEST_EQ("1.4426950408889634073599246810018921"_DL, log2e_v<decimal128_t>);
    BOOST_TEST_EQ("0.43429448190325182765112891891660508"_DL, log10e_v<decimal128_t>);
    BOOST_TEST_EQ("3.1415926535897932384626433832795029"_DL, pi_v<decimal128_t>);
    BOOST_TEST_EQ("0.31830988618379067153776752674502872"_DL, inv_pi_v<decimal128_t>);
    BOOST_TEST_EQ("0.56418958354775628694807945156077259"_DL, inv_sqrtpi_v<decimal128_t>);
    BOOST_TEST_EQ("0.69314718055994530941723212145817657"_DL, ln2_v<decimal128_t>);
    BOOST_TEST_EQ("2.3025850929940456840179914546843642"_DL, ln10_v<decimal128_t>);
    BOOST_TEST_EQ("1.4142135623730950488016887242096981"_DL, sqrt2_v<decimal128_t>);
    BOOST_TEST_EQ("1.7320508075688772935274463415058724"_DL, sqrt3_v<decimal128_t>);
    BOOST_TEST_EQ("0.70710678118654752440084436210484904"_DL, inv_sqrt2_v<decimal128_t>);
    BOOST_TEST_EQ("0.57735026918962576450914878050195746"_DL, inv_sqrt3_v<decimal128_t>);
    BOOST_TEST_EQ("0.57721566490153286060651209008240243"_DL, egamma_v<decimal128_t>);
    BOOST_TEST_EQ("1.6180339887498948482045868343656381"_DL, phi_v<decimal128_t>);

    BOOST_TEST_EQ(static_cast<decimal64_t>(e_v<decimal128_t>), e_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(log2e_v<decimal128_t>), log2e_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(log10e_v<decimal128_t>), log10e_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(pi_v<decimal128_t>), pi_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(inv_pi_v<decimal128_t>), inv_pi_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(inv_sqrtpi_v<decimal128_t>), inv_sqrtpi_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(ln2_v<decimal128_t>), ln2_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(ln10_v<decimal128_t>), ln10_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(sqrt2_v<decimal128_t>), sqrt2_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(sqrt3_v<decimal128_t>), sqrt3_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(inv_sqrt2_v<decimal128_t>), inv_sqrt2_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(inv_sqrt3_v<decimal128_t>), inv_sqrt3_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(egamma_v<decimal128_t>), egamma_v<decimal64_t>);
    BOOST_TEST_EQ(static_cast<decimal64_t>(phi_v<decimal128_t>), phi_v<decimal64_t>);
}

void test_defaults()
{
    BOOST_TEST_EQ(decimal64_t(2.718281828459045235), e);
    BOOST_TEST_EQ(decimal64_t(1.442695040888963407), log2e);
    BOOST_TEST_EQ(decimal64_t(0.4342944819032518277), log10e);
    BOOST_TEST_EQ(decimal64_t(3.141592653589793238), pi);
    BOOST_TEST_EQ(decimal64_t(0.3183098861837906715), inv_pi);
    BOOST_TEST_EQ(decimal64_t(0.5641895835477562869), inv_sqrtpi);
    BOOST_TEST_EQ(decimal64_t(0.6931471805599453094), ln2);
    BOOST_TEST_EQ(decimal64_t(2.302585092994045684), ln10);
    BOOST_TEST_EQ(decimal64_t(1.414213562373095049), sqrt2);
    BOOST_TEST_EQ(decimal64_t(1.732050807568877294), sqrt3);
    BOOST_TEST(abs(decimal64_t(0.707106781186547524) - inv_sqrt2_v<decimal64_t>) <= std::numeric_limits<decimal64_t>::epsilon());
    BOOST_TEST(abs(decimal64_t(0.5773502691896257645) - inv_sqrt3_v<decimal64_t>) <= std::numeric_limits<decimal64_t>::epsilon());
    BOOST_TEST_EQ(decimal64_t(0.5772156649015328606), egamma);
    BOOST_TEST_EQ(decimal64_t(1.618033988749894848), phi);
}

int main()
{
    test_constants<decimal32_t>();
    test_constants<decimal_fast32_t>();
    test_constants<decimal64_t>();
    test_constants<decimal128_t>();
    test_defaults();

    #ifdef BOOST_DECIMAL_GENERATE_CONSTANT_SIGS
    print_value("2.718281828459045235360287471352662"_DL, "e");
    print_value("1.4426950408889634073599246810018921"_DL, "log2");
    print_value("0.43429448190325182765112891891660508"_DL, "log10");
    print_value("3.1415926535897932384626433832795029"_DL, "pi");
    print_value("0.31830988618379067153776752674502872"_DL, "inv_pi");
    print_value("0.56418958354775628694807945156077259"_DL, "inv_sqrt_pi");
    print_value("0.69314718055994530941723212145817657"_DL, "ln2");
    print_value("2.3025850929940456840179914546843642"_DL, "ln10");
    print_value("1.4142135623730950488016887242096981"_DL, "sqrt(2)");
    print_value("1.7320508075688772935274463415058724"_DL, "sqrt(3)");
    print_value("0.70710678118654752440084436210484904"_DL, "1/sqrt(2)");
    print_value("0.57735026918962576450914878050195746"_DL, "1/sqrt(3)");
    print_value("0.57721566490153286060651209008240243"_DL, "egamma");
    print_value("1.6180339887498948482045868343656381"_DL, "phi");
    #endif

    return boost::report_errors();
}
