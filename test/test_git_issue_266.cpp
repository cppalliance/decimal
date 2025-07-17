// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

template <typename T>
void test()
{
    T false_val(false);
    BOOST_TEST(!false_val);
    T true_val(true);
    BOOST_TEST(true_val);
    BOOST_TEST(true_val != false_val);
}

int main()
{
    test<decimal32>();
    test<decimal64>();
    test<decimal128>();

    test<decimal32_fast>();
    test<decimal64_fast>();
    test<decimal128_fast>();

    return boost::report_errors();
}
