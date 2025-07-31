// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <limits>
#include <cmath>

int error_counter = 0;

template <typename T>
bool float_equal(T lhs, T rhs)
{
    using std::fabs;
    return fabs(lhs - rhs) < std::numeric_limits<T>::epsilon(); // numeric_limits is overloaded for all decimal types
}

template <typename T>
void test(T val)
{
    using std::sin; // ADL allows builtin and decimal types to both be used
    if (!float_equal(sin(val), -sin(-val))) // sin(x) == -sin(-x)
    {
        ++error_counter;
    }
}

int main()
{
    test(-0.5F);
    test(-0.5);
    test(-0.5L);

    test(boost::decimal::decimal32_t{-5, -1});
    test(boost::decimal::decimal64_t{-5, -1});
    test(boost::decimal::decimal128_t{-5, -1});

    return error_counter;
}


