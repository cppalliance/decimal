// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// This is a toy example to make sure that the hashing compiles correctly

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/decimal32_fast.hpp>
#include <boost/decimal/decimal64_fast.hpp>
#include <boost/decimal/decimal128_fast.hpp>
#include <boost/decimal/hash.hpp>
#include <boost/decimal/iostream.hpp>
#include <boost/core/lightweight_test.hpp>
#include <functional>

template <typename T>
void test_hash()
{
    std::hash<T> hasher;
    for (int i = 0; i < 100; ++i)
    {
        T dec_val(i);
        BOOST_TEST_EQ(hasher(dec_val), hasher(dec_val));
    }
}

int main()
{
    test_hash<boost::decimal::decimal32>();
    test_hash<boost::decimal::decimal64>();
    test_hash<boost::decimal::decimal128>();
    test_hash<boost::decimal::decimal32_fast>();
    test_hash<boost::decimal::decimal64_fast>();
    test_hash<boost::decimal::decimal128_fast>();

    return boost::report_errors();
}
