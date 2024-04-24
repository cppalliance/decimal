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

using namespace boost::decimal;

template <typename T>
void finite()
{
    BOOST_TEST_EQ(copysign(T(7.50), T(11)), T(7.50));
    BOOST_TEST_EQ(copysign(T(1.50), T(7.33)), T(1.50));
    BOOST_TEST_EQ(copysign(T(-1.50), T(7.33)), T(1.50));
    BOOST_TEST_EQ(copysign(T(1.50), T(-7.33)), T(-1.50));
    BOOST_TEST_EQ(copysign(T(-1.50), T(-7.33)), T(-1.50));

    BOOST_TEST_EQ(copysign(T(7), T(11)), T(7));
    BOOST_TEST_EQ(copysign(T(-7), T(11)), T(7));
    BOOST_TEST_EQ(copysign(T(75), T(11)), T(75));
    BOOST_TEST_EQ(copysign(T(-75), T(11)), T(75));
    BOOST_TEST_EQ(copysign(T(7.50), T(11)), T(7.50));
    BOOST_TEST_EQ(copysign(T(-7.50), T(11)), T(7.50));
    BOOST_TEST_EQ(copysign(T(7.500), T(11)), T(7.500));
    BOOST_TEST_EQ(copysign(T(-7.500), T(11)), T(7.500));

    BOOST_TEST_EQ(copysign(T(268268268), T(11)), T(268268268));
    BOOST_TEST_EQ(copysign(T(-268268268), T(11)), T(268268268));
    BOOST_TEST_EQ(copysign(T(134134134), T(11)), T(134134134));
    BOOST_TEST_EQ(copysign(T(-134134134), T(11)), T(134134134));

    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::max(), T(11)), std::numeric_limits<T>::max());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::max(), T(11)), std::numeric_limits<T>::max());
    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::min(), T(11)), std::numeric_limits<T>::min());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::min(), T(11)), std::numeric_limits<T>::min());
    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::epsilon(), T(11)), std::numeric_limits<T>::epsilon());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::epsilon(), T(11)), std::numeric_limits<T>::epsilon());

    BOOST_TEST_EQ(copysign(T(7), T(-11)), T(-7));
    BOOST_TEST_EQ(copysign(T(-7), T(-11)), T(-7));
    BOOST_TEST_EQ(copysign(T(75), T(-11)), T(-75));
    BOOST_TEST_EQ(copysign(T(-75), T(-11)), T(-75));
    BOOST_TEST_EQ(copysign(T(7.50), T(-11)), T(-7.50));
    BOOST_TEST_EQ(copysign(T(-7.50), T(-11)), T(-7.50));
    BOOST_TEST_EQ(copysign(T(7.500), T(-11)), T(-7.500));
    BOOST_TEST_EQ(copysign(T(-7.500), T(-11)), T(-7.500));

    BOOST_TEST_EQ(copysign(T(268268268), T(-11)), T(-268268268));
    BOOST_TEST_EQ(copysign(T(-268268268), T(-11)), T(-268268268));
    BOOST_TEST_EQ(copysign(T(134134134), T(-11)), T(-134134134));
    BOOST_TEST_EQ(copysign(T(-134134134), T(-11)), T(-134134134));

    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::max(), T(-11)), -std::numeric_limits<T>::max());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::max(), T(-11)), -std::numeric_limits<T>::max());
    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::min(), T(-11)), -std::numeric_limits<T>::min());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::min(), T(-11)), -std::numeric_limits<T>::min());
    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::epsilon(), T(-11)), -std::numeric_limits<T>::epsilon());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::epsilon(), T(-11)), -std::numeric_limits<T>::epsilon());

    BOOST_TEST_EQ(copysign(T(701), T(-34)), T(-701));
    BOOST_TEST_EQ(copysign(T(-720), T(-34)), T(-720));
    BOOST_TEST_EQ(copysign(T(701), -T(0)), T(-701));
    BOOST_TEST_EQ(copysign(T(-720), -T(0)), T(-720));
    BOOST_TEST_EQ(copysign(T(701), T(0)), T(701));
    BOOST_TEST_EQ(copysign(T(-720), T(0)), T(720));

    BOOST_TEST_EQ(copysign(T(701), -std::numeric_limits<T>::infinity()), T(-701));
    BOOST_TEST_EQ(copysign(T(-720), -std::numeric_limits<T>::infinity()), T(-720));
    BOOST_TEST_EQ(copysign(T(701), std::numeric_limits<T>::infinity()), T(701));
    BOOST_TEST_EQ(copysign(T(-720), std::numeric_limits<T>::infinity()), T(720));

    BOOST_TEST_EQ(copysign(T(701), -std::numeric_limits<T>::quiet_NaN()), T(-701));
    BOOST_TEST_EQ(copysign(T(-720), -std::numeric_limits<T>::quiet_NaN()), T(-720));
    BOOST_TEST_EQ(copysign(T(701), std::numeric_limits<T>::quiet_NaN()), T(701));
    BOOST_TEST_EQ(copysign(T(-720), std::numeric_limits<T>::quiet_NaN()), T(720));

    BOOST_TEST_EQ(copysign(T(701), -std::numeric_limits<T>::signaling_NaN()), T(-701));
    BOOST_TEST_EQ(copysign(T(-720), -std::numeric_limits<T>::signaling_NaN()), T(-720));
    BOOST_TEST_EQ(copysign(T(701), std::numeric_limits<T>::signaling_NaN()), T(701));
    BOOST_TEST_EQ(copysign(T(-720), std::numeric_limits<T>::signaling_NaN()), T(720));
}

template <typename T>
void non_finite()
{
    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::infinity(), T(11)), std::numeric_limits<T>::infinity());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::infinity(), T(11)), std::numeric_limits<T>::infinity());

    BOOST_TEST(isnan(copysign(std::numeric_limits<T>::quiet_NaN(), T(11))));
    BOOST_TEST(!signbit(copysign(std::numeric_limits<T>::quiet_NaN(), T(11))));
    BOOST_TEST(isnan(copysign(-std::numeric_limits<T>::quiet_NaN(), T(11))));
    BOOST_TEST(!signbit(copysign(-std::numeric_limits<T>::quiet_NaN(), T(11))));
    BOOST_TEST(isnan(copysign(std::numeric_limits<T>::signaling_NaN(), T(11))));
    BOOST_TEST(!signbit(copysign(std::numeric_limits<T>::signaling_NaN(), T(11))));
    BOOST_TEST(isnan(copysign(-std::numeric_limits<T>::signaling_NaN(), T(11))));
    BOOST_TEST(!signbit(copysign(-std::numeric_limits<T>::signaling_NaN(), T(11))));

    BOOST_TEST_EQ(copysign(T(0), T(11)), T(0));
    BOOST_TEST_EQ(copysign(T(-0), T(11)), T(0));
    BOOST_TEST_EQ(copysign(T{0, 4}, T{11}), (T{0, 4}));
    BOOST_TEST_EQ(copysign(T{0, -141}, T{11}), (T{0, -141}));

    BOOST_TEST_EQ(copysign(std::numeric_limits<T>::infinity(), T(-11)), -std::numeric_limits<T>::infinity());
    BOOST_TEST_EQ(copysign(-std::numeric_limits<T>::infinity(), T(-11)), -std::numeric_limits<T>::infinity());

    BOOST_TEST(isnan(copysign(std::numeric_limits<T>::quiet_NaN(), T(-11))));
    BOOST_TEST(signbit(copysign(std::numeric_limits<T>::quiet_NaN(), T(-11))));
    BOOST_TEST(isnan(copysign(-std::numeric_limits<T>::quiet_NaN(), T(-11))));
    BOOST_TEST(signbit(copysign(-std::numeric_limits<T>::quiet_NaN(), T(-11))));
    BOOST_TEST(isnan(copysign(std::numeric_limits<T>::signaling_NaN(), T(-11))));
    BOOST_TEST(signbit(copysign(std::numeric_limits<T>::signaling_NaN(), T(-11))));
    BOOST_TEST(isnan(copysign(-std::numeric_limits<T>::signaling_NaN(), T(-11))));
    BOOST_TEST(signbit(copysign(-std::numeric_limits<T>::signaling_NaN(), T(-11))));

    BOOST_TEST_EQ(copysign(T(0), T(11)), T(0));
    BOOST_TEST_EQ(copysign(T(-0), T(11)), T(0));
    BOOST_TEST_EQ(copysign(T{0, 4}, T{11}), (T{0, 4}));
    BOOST_TEST_EQ(copysign(T{0, -141}, T{11}), (T{0, -141}));

    BOOST_TEST_EQ(copysign(T(0), T(-11)), -T(0));
    BOOST_TEST_EQ(copysign(T(-0), T(-11)), -T(0));
    BOOST_TEST_EQ(copysign(T{0, 4}, T{-11}), (T{0, 4, true}));
    BOOST_TEST_EQ(copysign(T{0, -141}, T{-11}), (T{0, -141, true}));
}

int main()
{
    finite<decimal32>();
    finite<decimal64>();
    finite<decimal128>();

    non_finite<decimal32>();
    non_finite<decimal64>();
    non_finite<decimal128>();

    return boost::report_errors();
}
