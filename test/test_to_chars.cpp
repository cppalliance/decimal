// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/math/special_functions/next.hpp>
#include <boost/charconv.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include <cwchar>

using namespace boost::decimal;

static std::mt19937_64 rng(42);

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(1024U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(1024U >> 4U); // Number of trials
#endif

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

template <typename T>
void test_value(T val, const char* result)
{
    char buffer[256] {};
    auto r = to_chars(buffer, buffer + sizeof(buffer), val);
    *r.ptr = '\0';
    BOOST_TEST(r);
    BOOST_TEST_CSTR_EQ(result, buffer);
}

template <typename T>
void test_non_finite_values()
{
    test_value(std::numeric_limits<T>::quiet_NaN(), "nan");
    test_value(-std::numeric_limits<T>::quiet_NaN(), "-nan");
    test_value(std::numeric_limits<T>::signaling_NaN(), "nan(snan)");
    test_value(-std::numeric_limits<T>::signaling_NaN(), "-nan(snan)");
    test_value(std::numeric_limits<T>::infinity(), "inf");
    test_value(-std::numeric_limits<T>::infinity(), "-inf");
}

int main()
{
    test_non_finite_values<decimal32>();
    test_non_finite_values<decimal64>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test_non_finite_values<decimal128>();
    #endif

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
