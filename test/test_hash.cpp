// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// This is a toy example to make sure that the hashing compiles correctly

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <functional>

template <typename T>
void test_hash()
{
    std::hash<T> hasher;
    for (int i = 0; i < 100; ++i)
    {
        T dec_val(i);
        BOOST_TEST_NE(hasher(dec_val), static_cast<std::size_t>(0));
    }
}

int main()
{
    test_hash<boost::decimal::decimal32>();
    test_hash<boost::decimal::decimal64>();
    test_hash<boost::decimal::decimal128>();

    return boost::report_errors();
}
