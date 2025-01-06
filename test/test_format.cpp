// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>

using namespace boost::decimal;

#ifdef BOOST_CRYPT_HAS_FORMAT_SUPPORT

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
void test_general()
{
    // For unknown reasons Clang does not like this empty bracket and throws compiler errors
    #ifndef __clang__

    BOOST_TEST_EQ(std::format("{}", T{1}), "1");
    BOOST_TEST_EQ(std::format("{}", T{10}), "10");
    BOOST_TEST_EQ(std::format("{}", T{100}), "100");
    BOOST_TEST_EQ(std::format("{}", T{1000}), "1000");
    BOOST_TEST_EQ(std::format("{}", T{10000}), "10000");
    BOOST_TEST_EQ(std::format("{}", T{210000}), "210000");
    BOOST_TEST_EQ(std::format("{}", T{2100000}), "2100000");

    if constexpr (std::numeric_limits<T>::digits10 <= 7)
    {
        BOOST_TEST_EQ(std::format("{}", T {21, 6}), "2.1e+07");
        BOOST_TEST_EQ(std::format("{}", T {211, 6}), "2.11e+08");
        BOOST_TEST_EQ(std::format("{}", T {2111, 6}), "2.111e+09");
    }
    else
    {
        BOOST_TEST_EQ(std::format("{}", T {21, 6}), "21000000");
        BOOST_TEST_EQ(std::format("{}", T {211, 6}), "211000000");
        BOOST_TEST_EQ(std::format("{}", T {2111, 6}), "2111000000");
    }

    BOOST_TEST_EQ(std::format("{}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(std::format("{}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(std::format("{}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(std::format("{}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(std::format("{}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(std::format("{}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");

    #endif // defined(__clang__)

    BOOST_TEST_EQ(std::format("{:g}", T{1}), "1");
    BOOST_TEST_EQ(std::format("{:g}", T{10}), "10");
    BOOST_TEST_EQ(std::format("{:g}", T{100}), "100");
    BOOST_TEST_EQ(std::format("{:g}", T{1000}), "1000");
    BOOST_TEST_EQ(std::format("{:g}", T{10000}), "10000");
    BOOST_TEST_EQ(std::format("{:g}", T{210000}), "210000");
    BOOST_TEST_EQ(std::format("{:g}", T{2100000}), "2100000");

    if constexpr (std::numeric_limits<T>::digits10 <= 7)
    {
        BOOST_TEST_EQ(std::format("{:g}", T {21, 6, true}), "-2.1e+07");
        BOOST_TEST_EQ(std::format("{:g}", T {211, 6, true}), "-2.11e+08");
        BOOST_TEST_EQ(std::format("{:g}", T {2111, 6, true}), "-2.111e+09");
    }
    else
    {
        BOOST_TEST_EQ(std::format("{:g}", T {21, 6, true}), "-21000000");
        BOOST_TEST_EQ(std::format("{:g}", T {211, 6, true}), "-211000000");
        BOOST_TEST_EQ(std::format("{:g}", T {2111, 6, true}), "-2111000000");
    }

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
    test_general<decimal32_fast>();
    test_general<decimal64>();
    test_general<decimal64_fast>();
    test_general<decimal128>();
    test_general<decimal128_fast>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
