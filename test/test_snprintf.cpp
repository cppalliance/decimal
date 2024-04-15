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
void test_bootstrap()
{
    constexpr std::size_t N = 128;
    const char* format = std::is_same<T, decimal32>::value ? "%H" :
                         std::is_same<T, decimal64>::value ? "%D" : "%DD";

    const char* general_format = std::is_same<T, decimal32>::value ? "%Hg" :
                                 std::is_same<T, decimal64>::value ? "%Dg" : "%DDg";

    const char* three_digit_format = std::is_same<T, decimal32>::value ? "%.3H" :
                                     std::is_same<T, decimal64>::value ? "%.3D" : "%.3DD";

    std::mt19937_64 rng(42);

    for (std::size_t i {}; i < N; ++i)
    {
        // General test
        test(T{rng()}, format);
        test(T{rng()}, general_format);

        // Generate number with only 3 digits
        test(T{rng()}, three_digit_format, chars_format::general, 3);
    }
}

int main()
{
    test_bootstrap<decimal32>();
    test_bootstrap<decimal64>();
    test_bootstrap<decimal128>();

    return boost::report_errors();
}
