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
    BOOST_TEST_EQ(Dec(0.5773502691896257645), inv_sqrt3_v<Dec>);
    BOOST_TEST_EQ(Dec(0.5772156649015328606), egamma_v<Dec>);
    BOOST_TEST_EQ(Dec(1.618033988749894848), phi_v<Dec>);
}

void test_defaults()
{
    BOOST_TEST_EQ(decimal32(2.718281828459045235), e);
    BOOST_TEST_EQ(decimal32(1.442695040888963407), log2e);
    BOOST_TEST_EQ(decimal32(0.4342944819032518277), log10e);
    BOOST_TEST_EQ(decimal32(3.141592653589793238), pi);
    BOOST_TEST_EQ(decimal32(0.3183098861837906715), inv_pi);
    BOOST_TEST_EQ(decimal32(0.5641895835477562869), inv_sqrtpi);
    BOOST_TEST_EQ(decimal32(0.6931471805599453094), ln2);
    BOOST_TEST_EQ(decimal32(2.302585092994045684), ln10);
    BOOST_TEST_EQ(decimal32(1.414213562373095049), sqrt2);
    BOOST_TEST_EQ(decimal32(1.732050807568877294), sqrt3);
    BOOST_TEST_EQ(decimal32(0.5773502691896257645), inv_sqrt3);
    BOOST_TEST_EQ(decimal32(0.5772156649015328606), egamma);
    BOOST_TEST_EQ(decimal32(1.618033988749894848), phi);
}

int main()
{
    test_constants<decimal32>();
    test_defaults();

    return boost::report_errors();
}
