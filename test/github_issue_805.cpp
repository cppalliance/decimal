// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>

using namespace boost::decimal;

template <typename Dec>
void test()
{
    Dec a(2, std::numeric_limits<Dec>::max_exponent10);
    Dec b = scalbln(a, 10);

    BOOST_TEST(isinf(b));
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

