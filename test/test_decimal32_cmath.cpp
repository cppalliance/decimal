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
    BOOST_TEST(isnan(floor(BOOST_DECIMAL_DEC_NAN)));
    BOOST_TEST(isnan(floor(-BOOST_DECIMAL_DEC_NAN)));
    BOOST_TEST(isinf(floor(BOOST_DECIMAL_DEC_INFINITY)));
    BOOST_TEST(isinf(floor(-BOOST_DECIMAL_DEC_INFINITY)));
    BOOST_TEST_EQ(floor(decimal32(0, 0)), decimal32(0, 0));
    BOOST_TEST_EQ(floor(decimal32(-0, 0)), decimal32(-0, 0));

    BOOST_TEST_EQ(floor(decimal32(27, -1)), decimal32(2, 0));
    BOOST_TEST_EQ(floor(decimal32(-27, -1)), decimal32(-3, 0));
    BOOST_TEST_EQ(floor(decimal32(27777, -4)), decimal32(2, 0));
    BOOST_TEST_EQ(floor(decimal32(-27777, -4)), decimal32(-3, 0));
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

    return boost::report_errors();
}
