// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include <climits>
#include <cerrno>
#include <limits>
#include <string>
#include <algorithm>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wundef"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wsign-conversion"
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4244) //Implicit conversion from char to int in toupper
#endif

#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

template <typename T>
void test(T value, const char* format_sprintf, chars_format fmt = chars_format::general, int precision = 6)
{
    char buffer[256];
    errno = 0;

    int num_bytes = boost::decimal::snprintf(buffer, sizeof(buffer), format_sprintf, value);

    BOOST_TEST_EQ(errno, 0);

    char charconv_buffer[256];
    auto r = to_chars(charconv_buffer, charconv_buffer + sizeof(charconv_buffer), value, fmt, precision);
    BOOST_TEST(r);
    *r.ptr = '\0';

    BOOST_TEST_CSTR_EQ(buffer, charconv_buffer);
    BOOST_TEST_EQ(num_bytes, r.ptr - charconv_buffer);
}

template <typename T>
void test_uppercase(T value, const char* format_sprintf, chars_format fmt = chars_format::general, int precision = 6)
{
    char buffer[256];
    errno = 0;

    int num_bytes = boost::decimal::snprintf(buffer, sizeof(buffer), format_sprintf, value);

    BOOST_TEST_EQ(errno, 0);

    char charconv_buffer[256];
    auto r = to_chars(charconv_buffer, charconv_buffer + sizeof(charconv_buffer), value, fmt, precision);
    BOOST_TEST(r);
    *r.ptr = '\0';

    std::string str {charconv_buffer};
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);

    BOOST_TEST_CSTR_EQ(buffer, str.c_str());
    BOOST_TEST_EQ(num_bytes, r.ptr - charconv_buffer);
}

template <typename T>
void test_bootstrap()
{
    constexpr std::size_t N = 128;
    const char* format = std::is_same<T, decimal32>::value ? "%H" :
                         std::is_same<T, decimal64>::value ? "%D" : "%DD";

    const char* general_format = std::is_same<T, decimal32>::value ? "%Hg" :
                                 std::is_same<T, decimal64>::value ? "%Dg" : "%DDg";

    const char* general_upper_format = std::is_same<T, decimal32>::value ? "%HG" :
                                       std::is_same<T, decimal64>::value ? "%DG" : "%DDG";

    const char* three_digit_format = std::is_same<T, decimal32>::value ? "%.3H" :
                                     std::is_same<T, decimal64>::value ? "%.3D" : "%.3DD";

    const char* scientific_format = std::is_same<T, decimal32>::value ? "%He" :
                                    std::is_same<T, decimal64>::value ? "%De" : "%DDe";

    const char* four_digit_scientific_format = std::is_same<T, decimal32>::value ? "%.4He" :
                                               std::is_same<T, decimal64>::value ? "%.4De" : "%.4DDe";

    const char* four_digit_upper_sci_format =  std::is_same<T, decimal32>::value ? "%.5HE" :
                                               std::is_same<T, decimal64>::value ? "%.5DE" : "%.5DDE";

    const char* fixed_format = std::is_same<T, decimal32>::value ? "%Hf" :
                               std::is_same<T, decimal64>::value ? "%Df" : "%DDf";

    const char* two_digit_fixed_format = std::is_same<T, decimal32>::value ? "%.2Hf" :
                                         std::is_same<T, decimal64>::value ? "%.2Df" : "%.2DDf";

    const char* hex_format = std::is_same<T, decimal32>::value ? "%Ha" :
                             std::is_same<T, decimal64>::value ? "%Da" : "%DDa";

    const char* hex_upper_format = std::is_same<T, decimal32>::value ? "%HA" :
                                   std::is_same<T, decimal64>::value ? "%DA" : "%DDA";

    const char* one_digit_hex_upper_format = std::is_same<T, decimal32>::value ? "%.1HA" :
                                             std::is_same<T, decimal64>::value ? "%.1DA" : "%.1DDA";

    std::mt19937_64 rng(42);

    for (std::size_t i {}; i < N; ++i)
    {
        // General test
        test(T{rng()}, format);
        test(T{rng()}, general_format);
        test(T{rng()}, three_digit_format, chars_format::general, 3);
        test_uppercase(T{rng()}, general_upper_format);

        // Scientific format
        test(T{rng()}, scientific_format, chars_format::scientific);
        test(T{rng()}, four_digit_scientific_format, chars_format::scientific, 4);
        test_uppercase(T{rng()}, four_digit_upper_sci_format, chars_format::scientific, 5);

        // Fixed
        test(T{rng()}, fixed_format, chars_format::fixed);
        test(T{rng()}, two_digit_fixed_format, chars_format::fixed, 2);

        // Hex
        test(T{rng()}, hex_format, chars_format::hex);
        test_uppercase(T{rng()}, hex_upper_format, chars_format::hex);
        test_uppercase(T{rng()}, one_digit_hex_upper_format, chars_format::hex, 1);
    }
}

int main()
{
    test_bootstrap<decimal32>();
    test_bootstrap<decimal64>();
    test_bootstrap<decimal128>();

    return boost::report_errors();
}
