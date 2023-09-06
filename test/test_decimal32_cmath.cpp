// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <cmath>

using namespace boost::decimal;

template <typename Dec>
void test_fmax()
{
    BOOST_TEST_EQ(fmax(Dec(1), BOOST_DECIMAL_DEC_NAN), Dec(1));
    BOOST_TEST_EQ(fmax(BOOST_DECIMAL_DEC_NAN, Dec(1)), Dec(1));
    BOOST_TEST(isnan(fmax(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN)));
    BOOST_TEST_EQ(fmax(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), BOOST_DECIMAL_DEC_INFINITY);

    BOOST_TEST_EQ(fmax(Dec(1), Dec(0)), Dec(1));
    BOOST_TEST_EQ(fmax(Dec(-2), Dec(1)), Dec(1));
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

int main()
{
    test_fmax<decimal32>();
    test_fmin<decimal32>();

    return boost::report_errors();
}
