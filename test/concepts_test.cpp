// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

template <typename T>
void test()
{
    T res = T(1.0) + 1.0;
    BOOST_TEST_EQ(res, T(2.0));
    BOOST_TEST(boost::decimal::abs(1.0));
}


int main()
{
    test<boost::decimal::decimal32>();
    //test<boost::decimal::decimal64>();
    //test<boost::decimal::decimal128>();

    return boost::report_errors();
}

