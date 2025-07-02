// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

template <typename From, typename To>
void test_implicit()
{
    const From from_val {2, 1};
    const To to_val = from_val;

    BOOST_TEST_EQ(from_val, to_val);
}

int main()
{
    test_implicit<boost::decimal::decimal128, boost::decimal::decimal64_fast>();
    test_implicit<boost::decimal::decimal128_fast, boost::decimal::decimal64_fast>();

    return boost::report_errors();
}
