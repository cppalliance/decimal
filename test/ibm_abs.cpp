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

template <typename T>
void test()
{
    BOOST_TEST_EQ(abs(T(1)), T(1));
    BOOST_TEST_EQ(abs(T(-1)), T(1));
    BOOST_TEST_EQ(abs(T(1.00)), T(1.00));
    BOOST_TEST_EQ(abs(T(-1.00)), T(1.00));
    BOOST_TEST_EQ(abs(T(0)), T(0.0));
    BOOST_TEST_EQ(abs(T(-0.0)), T(0.0));
    BOOST_TEST_EQ(abs(T(2)), T(2));
    BOOST_TEST_EQ(abs(T(-2)), T(2));
    BOOST_TEST_EQ(abs(T(2.00)), T(2.00));
    BOOST_TEST_EQ(abs(T(-2.00)), T(2.00));
    BOOST_TEST_EQ(abs(T(2000000)), T(2000000));
    BOOST_TEST_EQ(abs(T(-2000000)), T(2000000));

    BOOST_TEST_EQ(abs(T(0.1)), T(0.1));
    BOOST_TEST_EQ(abs(T(0.01)), T(0.01));
    BOOST_TEST_EQ(abs(T(0.001)), T(0.001));
    BOOST_TEST_EQ(abs(T(0.00001)), T(0.00001));
    BOOST_TEST_EQ(abs(T(0.000001)), T(0.000001));

    BOOST_TEST_EQ(abs(T(-0.1)), T(0.1));
    BOOST_TEST_EQ(abs(T(-0.01)), T(0.01));
    BOOST_TEST_EQ(abs(T(-0.001)), T(0.001));
    BOOST_TEST_EQ(abs(T(-0.00001)), T(0.00001));
    BOOST_TEST_EQ(abs(T(-0.000001)), T(0.000001));
    BOOST_TEST_EQ(abs(T(-0.000000000000000000001)), T(1, -21));

    BOOST_TEST_EQ(abs(T(2.1)), T(2.1));
    BOOST_TEST_EQ(abs(T(-100)), T(100));
    BOOST_TEST_EQ(abs(T(101.5)), T(101.5));
    BOOST_TEST_EQ(abs(T(-101.5)), T(101.5));
}

template <typename T>
void overflow_underflow_subnormals()
{
    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<T, boost::decimal::decimal32>::value)
    {
        BOOST_TEST(isinf(abs(T(9.999e+99))));
        BOOST_TEST_EQ(abs(T(0.1e-99)), T(0.0));
        BOOST_TEST_EQ(abs(T(-0.1e-99)), T(0.0));
    }
    #if (BOOST_DECIMAL_LDBL_BITS > 64)
    else BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<T, boost::decimal::decimal64>::value)
    {
        BOOST_TEST(isinf(abs(T(9.999e+999L))));
        BOOST_TEST_EQ(abs(T(0.1e-999L)), T(0.0L));
        BOOST_TEST_EQ(abs(T(-0.1e-999L)), T(0.0L));
    }
    #endif
}

template <typename T>
void non_finitie_values()
{
    BOOST_TEST_EQ(abs(T(std::numeric_limits<T>::infinity())), T(std::numeric_limits<T>::infinity()));
    BOOST_TEST(!signbit(abs(T(std::numeric_limits<T>::quiet_NaN()))));
    BOOST_TEST(!signbit(abs(T(-std::numeric_limits<T>::quiet_NaN()))));
    BOOST_TEST(!signbit(abs(T(std::numeric_limits<T>::signaling_NaN()))));
    BOOST_TEST(!signbit(abs(T(-std::numeric_limits<T>::signaling_NaN()))));
}


int main()
{
    test<boost::decimal::decimal32>();
    test<boost::decimal::decimal64>();

    overflow_underflow_subnormals<boost::decimal::decimal32>();
    overflow_underflow_subnormals<boost::decimal::decimal64>();

    non_finitie_values<boost::decimal::decimal32>();
    non_finitie_values<boost::decimal::decimal64>();

    return boost::report_errors();
}

