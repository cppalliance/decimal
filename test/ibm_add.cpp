// Copyright 2023 Matt Borland
// Copyright (c) Mike Cowlishaw,  1981, 2010.
// Parts copyright (c) IBM Corporation, 1981, 2008.
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// Derived from: https://speleotrove.com/decimal/dectest.html

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>
#include <iostream>
#include <iomanip>

template <typename T>
void test()
{
    std::cerr << std::setprecision(std::numeric_limits<T>::digits10);

    BOOST_TEST_EQ(T(1) + T(1), T(2));
    BOOST_TEST_EQ(T(2) + T(3), T(5));
    BOOST_TEST_EQ(T(5.75) + T(3.3), T(9.05));
    BOOST_TEST_EQ(T(5) + T(-2), T(3));
    BOOST_TEST_EQ(T(-5) + T(-3), T(-8));
    BOOST_TEST_EQ(T(-7) + T(2.5), T(-4.5));
    BOOST_TEST_EQ(T(0.7) + T(0.3), T(1.0));
    BOOST_TEST_EQ(T(1.25) + T(1.25), T(2.50));
    BOOST_TEST_EQ(T(1.23456789) + T(1.000000000), T(2.23456789));
    BOOST_TEST_EQ(T(1.23456) + T(1.00044), T(2.23500));
}

template <typename T>
void test_inexact()
{
    BOOST_TEST_EQ(T(0.444444444444444444444) + T(0.5555555555555555555555), T(1.00000000000000000));
    BOOST_TEST_EQ(T(0.444444444444444444449) + T(0.0), T(0.4444444444444444444444));
    BOOST_TEST_EQ(T(0.4444444444444444444499) + T(0.0), T(0.4444444444444444444444));
    BOOST_TEST_EQ(T(0.44444444444444444444999) + T(0.0), T(0.4444444444444444444444));

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<T, boost::decimal::decimal32>::value)
    {
        BOOST_TEST_EQ(T(0.4444449) + T(0.0), T(0.4444449));
        BOOST_TEST_EQ(T(0.4444449) + T(0.0000001), T(0.4444450));
        BOOST_TEST_EQ(T(0.4444449) + T(0.00000009), T(0.4444450));
    }

}

int main()
{
    test<boost::decimal::decimal32>();
    test<boost::decimal::decimal64>();

    test_inexact<boost::decimal::decimal32>();
    test_inexact<boost::decimal::decimal64>();

    return boost::report_errors();
}

