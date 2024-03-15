// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)) && __has_include(<format>) && !defined(BOOST_DECIMAL_DISABLE_CLIB)

#include <format>

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
void test_general()
{
    BOOST_TEST_EQ(std::format("{}", T{1}), "1");
    BOOST_TEST_EQ(std::format("{}", T{10}), "10");
    BOOST_TEST_EQ(std::format("{}", T{100}), "100");
    BOOST_TEST_EQ(std::format("{}", T{1000}), "1000");
    BOOST_TEST_EQ(std::format("{}", T{10000}), "10000");
    BOOST_TEST_EQ(std::format("{}", T{210000}), "210000");
    BOOST_TEST_EQ(std::format("{}", T{2100000}), "2100000");
    BOOST_TEST_EQ(std::format("{}", T{21, 6, true}), "-2.1e+07");
    BOOST_TEST_EQ(std::format("{}", T{211, 6, true}), "-2.11e+08");
    BOOST_TEST_EQ(std::format("{}", T{2111, 6, true}), "-2.111e+09");

    BOOST_TEST_EQ(std::format("{}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(std::format("{}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(std::format("{}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(std::format("{}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(std::format("{}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(std::format("{}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");

    BOOST_TEST_EQ(std::format("{:g}", T{1}), "1");
    BOOST_TEST_EQ(std::format("{:g}", T{10}), "10");
    BOOST_TEST_EQ(std::format("{:g}", T{100}), "100");
    BOOST_TEST_EQ(std::format("{:g}", T{1000}), "1000");
    BOOST_TEST_EQ(std::format("{:g}", T{10000}), "10000");
    BOOST_TEST_EQ(std::format("{:g}", T{210000}), "210000");
    BOOST_TEST_EQ(std::format("{:g}", T{2100000}), "2100000");
    BOOST_TEST_EQ(std::format("{:g}", T{21, 6, true}), "-2.1e+07");
    BOOST_TEST_EQ(std::format("{:g}", T{211, 6, true}), "-2.11e+08");
    BOOST_TEST_EQ(std::format("{:g}", T{2111, 6, true}), "-2.111e+09");

    BOOST_TEST_EQ(std::format("{:g}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(std::format("{:g}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(std::format("{:g}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(std::format("{:g}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(std::format("{:g}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(std::format("{:g}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");
}

int main()
{
    test_general<decimal32>();

    //test<decimal64>();
    //test<decimal128>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
