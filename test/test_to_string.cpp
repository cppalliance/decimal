// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
void test()
{
    BOOST_TEST_EQ(to_string(T{1}), "1");
    BOOST_TEST_EQ(to_string(T{10}), "1e+01");
    BOOST_TEST_EQ(to_string(T{100}), "1e+02");
    BOOST_TEST_EQ(to_string(T{1000}), "1e+03");
    BOOST_TEST_EQ(to_string(T{10000}), "1e+04");
    BOOST_TEST_EQ(to_string(T{210000}), "2.1e+05");
    BOOST_TEST_EQ(to_string(T{2100000}), "2.1e+06");
    BOOST_TEST_EQ(to_string(T{21, 6, true}), "-2.1e+07");
    BOOST_TEST_EQ(to_string(T{211, 6, true}), "-2.11e+08");
    BOOST_TEST_EQ(to_string(T{2111, 6, true}), "-2.111e+09");

    BOOST_TEST_EQ(to_string(std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(to_string(-std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(to_string(std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(to_string(-std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(to_string(std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(to_string(-std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");
}

int main()
{
    test<decimal32>();
    test<decimal64>();
    test<decimal128>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
