// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <cmath>

using namespace boost::decimal;

void test_fmax()
{
    BOOST_TEST_EQ(fmax(decimal32(1), BOOST_DECIMAL_DEC_NAN), decimal32(1));
    BOOST_TEST_EQ(fmax(BOOST_DECIMAL_DEC_NAN, decimal32(1)), decimal32(1));
    BOOST_TEST(isnan(fmax(BOOST_DECIMAL_DEC_NAN, BOOST_DECIMAL_DEC_NAN)));
    BOOST_TEST_EQ(fmax(BOOST_DECIMAL_DEC_INFINITY, -BOOST_DECIMAL_DEC_INFINITY), BOOST_DECIMAL_DEC_INFINITY);

    BOOST_TEST_EQ(fmax(decimal32(1), decimal32(0)), decimal32(1));
    BOOST_TEST_EQ(fmax(decimal32(-2), decimal32(1)), decimal32(1));
}

int main()
{
    test_fmax();

    return boost::report_errors();
}
