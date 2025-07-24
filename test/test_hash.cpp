// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// This is a toy example to make sure that the hashing compiles correctly

#include <boost/decimal/decimal32_t_t.hpp>
#include <boost/decimal/decimal64_t_t.hpp>
#include <boost/decimal/decimal128_t_t.hpp>
#include <boost/decimal/decimal_fast32_t.hpp>
#include <boost/decimal/decimal_fast64_t.hpp>
#include <boost/decimal/decimal_fast128_t.hpp>
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
    test_hash<boost::decimal::decimal32_t_t>();
    test_hash<boost::decimal::decimal64_t_t>();
    test_hash<boost::decimal::decimal128_t_t>();
    test_hash<boost::decimal::decimal_fast32_t>();
    test_hash<boost::decimal::decimal_fast64_t>();
    test_hash<boost::decimal::decimal_fast128_t>();

    return boost::report_errors();
}
