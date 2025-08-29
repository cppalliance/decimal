// Copyright 2023 - 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#define FMT_HEADER_ONLY
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>

using namespace boost::decimal;

#if defined(BOOST_DECIMAL_HAS_FMTLIB_SUPPORT)

#include <fmt/format.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127) // Conditional expression is constant
#endif

template <typename T>
void test_general()
{

    BOOST_TEST_EQ(fmt::format("{}", T{1}), "1");
    BOOST_TEST_EQ(fmt::format("{}", T{10}), "10");
    BOOST_TEST_EQ(fmt::format("{}", T{100}), "100");
    BOOST_TEST_EQ(fmt::format("{}", T{1000}), "1000");
    BOOST_TEST_EQ(fmt::format("{}", T{10000}), "10000");
    BOOST_TEST_EQ(fmt::format("{}", T{210000}), "210000");
    BOOST_TEST_EQ(fmt::format("{}", T{2100000}), "2100000");

    BOOST_DECIMAL_IF_CONSTEXPR (std::numeric_limits<T>::digits10 <= 7)
    {
        BOOST_TEST_EQ(fmt::format("{}", T {21u, 6}), "2.1e+07");
        BOOST_TEST_EQ(fmt::format("{}", T {211u, 6}), "2.11e+08");
        BOOST_TEST_EQ(fmt::format("{}", T {2111u, 6}), "2.111e+09");
    }
    else
    {
        BOOST_TEST_EQ(fmt::format("{}", T {21u, 6}), "21000000");
        BOOST_TEST_EQ(fmt::format("{}", T {211u, 6}), "211000000");
        BOOST_TEST_EQ(fmt::format("{}", T {2111u, 6}), "2111000000");
    }

    BOOST_TEST_EQ(fmt::format("{}", T{}), "0");
    BOOST_TEST_EQ(fmt::format("{}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(fmt::format("{}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(fmt::format("{}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(fmt::format("{}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(fmt::format("{}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(fmt::format("{}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");

    BOOST_TEST_EQ(fmt::format("{:g}", T{1}), "1");
    BOOST_TEST_EQ(fmt::format("{:g}", T{10}), "10");
    BOOST_TEST_EQ(fmt::format("{:g}", T{100}), "100");
    BOOST_TEST_EQ(fmt::format("{:g}", T{1000}), "1000");
    BOOST_TEST_EQ(fmt::format("{:g}", T{10000}), "10000");
    BOOST_TEST_EQ(fmt::format("{:g}", T{210000}), "210000");
    BOOST_TEST_EQ(fmt::format("{:g}", T{2100000}), "2100000");

    BOOST_TEST_EQ(fmt::format("{:+g}", T{1}), "+1");
    BOOST_TEST_EQ(fmt::format("{:+g}", T{10}), "+10");
    BOOST_TEST_EQ(fmt::format("{:+g}", T{100}), "+100");
    BOOST_TEST_EQ(fmt::format("{:+g}", T{1000}), "+1000");
    BOOST_TEST_EQ(fmt::format("{:+g}", T{10000}), "+10000");
    BOOST_TEST_EQ(fmt::format("{:+g}", T{210000}), "+210000");
    BOOST_TEST_EQ(fmt::format("{:+g}", T{2100000}), "+2100000");

    BOOST_TEST_EQ(fmt::format("{:-g}", T{1}), "1");
    BOOST_TEST_EQ(fmt::format("{:-g}", T{10}), "10");
    BOOST_TEST_EQ(fmt::format("{:-g}", T{100}), "100");
    BOOST_TEST_EQ(fmt::format("{:-g}", T{1000}), "1000");
    BOOST_TEST_EQ(fmt::format("{:-g}", T{10000}), "10000");
    BOOST_TEST_EQ(fmt::format("{:-g}", T{210000}), "210000");
    BOOST_TEST_EQ(fmt::format("{:-g}", T{2100000}), "2100000");

    BOOST_TEST_EQ(fmt::format("{: g}", T{1}), " 1");
    BOOST_TEST_EQ(fmt::format("{: g}", T{10}), " 10");
    BOOST_TEST_EQ(fmt::format("{: g}", T{100}), " 100");
    BOOST_TEST_EQ(fmt::format("{: g}", T{1000}), " 1000");
    BOOST_TEST_EQ(fmt::format("{: g}", T{10000}), " 10000");
    BOOST_TEST_EQ(fmt::format("{: g}", T{210000}), " 210000");
    BOOST_TEST_EQ(fmt::format("{: g}", T{2100000}), " 2100000");

    BOOST_DECIMAL_IF_CONSTEXPR (std::numeric_limits<T>::digits10 <= 7)
    {
        BOOST_TEST_EQ(fmt::format("{:g}", T {21u, 6, true}), "-2.1e+07");
        BOOST_TEST_EQ(fmt::format("{:g}", T {211u, 6, true}), "-2.11e+08");
        BOOST_TEST_EQ(fmt::format("{:g}", T {2111u, 6, true}), "-2.111e+09");
        BOOST_TEST_EQ(fmt::format("{:G}", T {21u, 6, true}), "-2.1E+07");
        BOOST_TEST_EQ(fmt::format("{:G}", T {211u, 6, true}), "-2.11E+08");
        BOOST_TEST_EQ(fmt::format("{:G}", T {2111u, 6, true}), "-2.111E+09");
    }
    else
    {
        BOOST_TEST_EQ(fmt::format("{:g}", T {21u, 6, true}), "-21000000");
        BOOST_TEST_EQ(fmt::format("{:g}", T {211u, 6, true}), "-211000000");
        BOOST_TEST_EQ(fmt::format("{:g}", T {2111u, 6, true}), "-2111000000");

        BOOST_TEST_EQ(fmt::format("{:-g}", T {21u, 6, true}), "-21000000");
        BOOST_TEST_EQ(fmt::format("{:-g}", T {211u, 6, true}), "-211000000");
        BOOST_TEST_EQ(fmt::format("{:-g}", T {2111u, 6, true}), "-2111000000");

        BOOST_TEST_EQ(fmt::format("{:+g}", T {21u, 6, true}), "-21000000");
        BOOST_TEST_EQ(fmt::format("{:+g}", T {211u, 6, true}), "-211000000");
        BOOST_TEST_EQ(fmt::format("{:+g}", T {2111u, 6, true}), "-2111000000");
    }

    BOOST_TEST_EQ(fmt::format("{:g}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(fmt::format("{:g}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(fmt::format("{:g}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(fmt::format("{:g}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(fmt::format("{:g}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(fmt::format("{:g}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");

    BOOST_TEST_EQ(fmt::format("{:G}", std::numeric_limits<T>::infinity()), "INF");
    BOOST_TEST_EQ(fmt::format("{:G}", -std::numeric_limits<T>::infinity()), "-INF");
    BOOST_TEST_EQ(fmt::format("{:G}", std::numeric_limits<T>::quiet_NaN()), "NAN");
    BOOST_TEST_EQ(fmt::format("{:G}", -std::numeric_limits<T>::quiet_NaN()), "-NAN(IND)");
    BOOST_TEST_EQ(fmt::format("{:G}", std::numeric_limits<T>::signaling_NaN()), "NAN(SNAN)");
    BOOST_TEST_EQ(fmt::format("{:G}", -std::numeric_limits<T>::signaling_NaN()), "-NAN(SNAN)");
}

template <typename T>
void test_fixed()
{
    BOOST_TEST_EQ(fmt::format("{:f}", T {21u, 6, true}), "-21000000.000000");
    BOOST_TEST_EQ(fmt::format("{:f}", T {211u, 6, true}), "-211000000.000000");
    BOOST_TEST_EQ(fmt::format("{:f}", T {2111u, 6, true}), "-2111000000.000000");

    BOOST_TEST_EQ(fmt::format("{:.0f}", T {21u, 6, true}), std::string{"-21000000"});
    BOOST_TEST_EQ(fmt::format("{:.0f}", T {211u, 6, true}), std::string{"-211000000"});
    BOOST_TEST_EQ(fmt::format("{:.0f}", T {2111u, 6, true}), std::string{"-2111000000"});

    BOOST_TEST_EQ(fmt::format("{:.1f}", T {21u, 6, true}), std::string{"-21000000.0"});
    BOOST_TEST_EQ(fmt::format("{:.1f}", T {211u, 6, true}), std::string{"-211000000.0"});
    BOOST_TEST_EQ(fmt::format("{:.1f}", T {2111u, 6, true}), std::string{"-2111000000.0"});

    BOOST_TEST_EQ(fmt::format("{:.0f}", T {0}), "0");
    BOOST_TEST_EQ(fmt::format("{:f}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(fmt::format("{:f}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(fmt::format("{:f}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(fmt::format("{:f}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(fmt::format("{:f}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(fmt::format("{:f}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");

    BOOST_TEST_EQ(fmt::format("{:F}", std::numeric_limits<T>::infinity()), "INF");
    BOOST_TEST_EQ(fmt::format("{:F}", -std::numeric_limits<T>::infinity()), "-INF");
    BOOST_TEST_EQ(fmt::format("{:F}", std::numeric_limits<T>::quiet_NaN()), "NAN");
    BOOST_TEST_EQ(fmt::format("{:F}", -std::numeric_limits<T>::quiet_NaN()), "-NAN(IND)");
    BOOST_TEST_EQ(fmt::format("{:F}", std::numeric_limits<T>::signaling_NaN()), "NAN(SNAN)");
    BOOST_TEST_EQ(fmt::format("{:F}", -std::numeric_limits<T>::signaling_NaN()), "-NAN(SNAN)");
}

template <typename T>
void test_scientific()
{
    BOOST_TEST_EQ(fmt::format("{:e}", T {21u, 6, true}), "-2.100000e+07");
    BOOST_TEST_EQ(fmt::format("{:e}", T {211u, 6, true}), "-2.110000e+08");
    BOOST_TEST_EQ(fmt::format("{:e}", T {2111u, 6, true}), "-2.111000e+09");

    BOOST_TEST_EQ(fmt::format("{:E}", T {21u, 6, true}), "-2.100000E+07");
    BOOST_TEST_EQ(fmt::format("{:E}", T {211u, 6, true}), "-2.110000E+08");
    BOOST_TEST_EQ(fmt::format("{:E}", T {2111u, 6, true}), "-2.111000E+09");

    BOOST_TEST_EQ(fmt::format("{:.0E}", T {0}), "0E+00");
    BOOST_TEST_EQ(fmt::format("{:e}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(fmt::format("{:e}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(fmt::format("{:e}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(fmt::format("{:e}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(fmt::format("{:e}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(fmt::format("{:e}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");

    BOOST_TEST_EQ(fmt::format("{:E}", std::numeric_limits<T>::infinity()), "INF");
    BOOST_TEST_EQ(fmt::format("{:E}", -std::numeric_limits<T>::infinity()), "-INF");
    BOOST_TEST_EQ(fmt::format("{:E}", std::numeric_limits<T>::quiet_NaN()), "NAN");
    BOOST_TEST_EQ(fmt::format("{:E}", -std::numeric_limits<T>::quiet_NaN()), "-NAN(IND)");
    BOOST_TEST_EQ(fmt::format("{:E}", std::numeric_limits<T>::signaling_NaN()), "NAN(SNAN)");
    BOOST_TEST_EQ(fmt::format("{:E}", -std::numeric_limits<T>::signaling_NaN()), "-NAN(SNAN)");

    // Padding to the front
    BOOST_TEST_EQ(fmt::format("{:10.1E}", T {0}), "0000.0E+00");
    BOOST_TEST_EQ(fmt::format("{:10.3E}", T {0}), "00.000E+00");

    BOOST_TEST_EQ(fmt::format("{:+10.1E}", T {0}), "+000.0E+00");
    BOOST_TEST_EQ(fmt::format("{:+10.3E}", T {0}), "+0.000E+00");
    BOOST_TEST_EQ(fmt::format("{: 10.1E}", T {0}), " 000.0E+00");
    BOOST_TEST_EQ(fmt::format("{: 10.3E}", T {0}), " 0.000E+00");
}

template <typename T>
void test_hex()
{
    BOOST_TEST_EQ(fmt::format("{:.0a}", T {0}), "0p+00");
    BOOST_TEST_EQ(fmt::format("{:.3A}", T {0}), "0.000P+00");
    BOOST_TEST_EQ(fmt::format("{:a}", std::numeric_limits<T>::infinity()), "inf");
    BOOST_TEST_EQ(fmt::format("{:a}", -std::numeric_limits<T>::infinity()), "-inf");
    BOOST_TEST_EQ(fmt::format("{:a}", std::numeric_limits<T>::quiet_NaN()), "nan");
    BOOST_TEST_EQ(fmt::format("{:a}", -std::numeric_limits<T>::quiet_NaN()), "-nan(ind)");
    BOOST_TEST_EQ(fmt::format("{:a}", std::numeric_limits<T>::signaling_NaN()), "nan(snan)");
    BOOST_TEST_EQ(fmt::format("{:a}", -std::numeric_limits<T>::signaling_NaN()), "-nan(snan)");

    BOOST_TEST_EQ(fmt::format("{:A}", std::numeric_limits<T>::infinity()), "INF");
    BOOST_TEST_EQ(fmt::format("{:A}", -std::numeric_limits<T>::infinity()), "-INF");
    BOOST_TEST_EQ(fmt::format("{:A}", std::numeric_limits<T>::quiet_NaN()), "NAN");
    BOOST_TEST_EQ(fmt::format("{:A}", -std::numeric_limits<T>::quiet_NaN()), "-NAN(IND)");
    BOOST_TEST_EQ(fmt::format("{:A}", std::numeric_limits<T>::signaling_NaN()), "NAN(SNAN)");
    BOOST_TEST_EQ(fmt::format("{:A}", -std::numeric_limits<T>::signaling_NaN()), "-NAN(SNAN)");
}

template <typename T>
void test_with_string()
{
    BOOST_TEST_EQ(std::format("Height is: {:.0f} meters", T {0}), "Height is: 0 meters");
    BOOST_TEST_EQ(std::format("Height is: {} meters", T {2}), "Height is: 2 meters");
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

int main()
{
    test_general<decimal32_t>();
    test_general<decimal_fast32_t>();
    test_general<decimal64_t>();
    test_general<decimal_fast64_t>();
    test_general<decimal128_t>();
    test_general<decimal_fast128_t>();

    test_fixed<decimal32_t>();
    test_fixed<decimal_fast32_t>();
    test_fixed<decimal64_t>();
    test_fixed<decimal_fast64_t>();
    test_fixed<decimal128_t>();
    test_fixed<decimal_fast128_t>();

    test_scientific<decimal32_t>();
    test_scientific<decimal_fast32_t>();
    test_scientific<decimal64_t>();
    test_scientific<decimal_fast64_t>();
    test_scientific<decimal128_t>();
    test_scientific<decimal_fast128_t>();

    test_hex<decimal32_t>();
    test_hex<decimal_fast32_t>();
    test_hex<decimal64_t>();
    test_hex<decimal_fast64_t>();
    test_hex<decimal128_t>();
    test_hex<decimal_fast128_t>();

    test_with_string<decimal32_t>();
    test_with_string<decimal_fast32_t>();
    test_with_string<decimal64_t>();
    test_with_string<decimal_fast64_t>();
    test_with_string<decimal128_t>();
    test_with_string<decimal_fast128_t>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
