// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/decimal32_fast.hpp>
#include <boost/decimal/decimal64_fast.hpp>
#include <boost/decimal/decimal128_fast.hpp>
#include <boost/decimal/charconv.hpp>
#include <boost/decimal/iostream.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <iomanip>
#include <random>

using namespace boost::decimal;

static std::mt19937_64 rng(42);

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(1024U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(1024U >> 4U); // Number of trials
#endif

// stringop overflow errors from gcc-13 and on with x86
#if !defined(BOOST_DECIMAL_DISABLE_CLIB) && !(defined(__GNUC__) && __GNUC__ >= 13 && !defined(__aarch64__))

template <typename T>
void test_value(T val, const char* result, chars_format fmt, int precision)
{
    char buffer[256] {};
    auto r = to_chars(buffer, buffer + sizeof(buffer), val, fmt, precision);
    *r.ptr = '\0';
    BOOST_TEST(r);
    BOOST_TEST_CSTR_EQ(result, buffer);
}

template <typename T>
void test_value(T val, const char* result, chars_format fmt)
{
    char buffer[256] {};
    auto r = to_chars(buffer, buffer + sizeof(buffer), val, fmt);
    *r.ptr = '\0';
    BOOST_TEST(r);
    BOOST_TEST_CSTR_EQ(result, buffer);
}

template <typename T>
void test_value(T val, const char* result)
{
    char buffer[boost::decimal::limits<T>::max_chars] {};
    auto r = to_chars(buffer, buffer + sizeof(buffer), val, chars_format::general);
    *r.ptr = '\0';
    BOOST_TEST(r);
    BOOST_TEST_CSTR_EQ(result, buffer);
}

template <typename T>
void test_error_value(const char* input, chars_format format, int precision = -1)
{
    T val;
    const auto r_from = from_chars(input, input + std::strlen(input), val, format);
    BOOST_TEST(r_from);
    char buffer[boost::decimal::limits<T>::max_chars] {};
    const auto r_to = to_chars(buffer, buffer + sizeof(buffer), val, format, precision);
    BOOST_TEST(r_to);
}

template <typename T>
void test_non_finite_values()
{
    std::uniform_real_distribution<float> dist(-1.0, 1.0);

    const auto formats = {chars_format::fixed, chars_format::scientific, chars_format::general, chars_format::hex};

    for (const auto format : formats)
    {
        test_value(std::numeric_limits<T>::quiet_NaN() * T {dist(rng)}, "nan", format);
        test_value(-std::numeric_limits<T>::quiet_NaN() * T {dist(rng)}, "-nan(ind)", format);
        test_value(std::numeric_limits<T>::signaling_NaN() * T {dist(rng)}, "nan(snan)", format);
        test_value(-std::numeric_limits<T>::signaling_NaN() * T {dist(rng)}, "-nan(snan)", format);
        test_value(std::numeric_limits<T>::infinity() * T {dist(rng)}, "inf", format);
        test_value(-std::numeric_limits<T>::infinity() * T {dist(rng)}, "-inf", format);
    }
}

template <typename T>
void test_small_values()
{
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256];

        const auto val {dist(rng)};
        const T dec_val {val};

        auto to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val);
        *to_r.ptr = '\0';
        BOOST_TEST(to_r);

        T ret_val;
        auto from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val);
        BOOST_TEST(from_r);

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nRet val:" << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
    
    test_value(T{0}, "0.0e+00");
}

template <typename T>
void test_large_values()
{
    constexpr double max_value = std::is_same<decimal32, T>::value || std::is_same<decimal32_fast, T>::value ? 1e80 : 1e200;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256];

        const auto val {dist(rng)};
        const T dec_val {val};

        auto to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val);
        *to_r.ptr = '\0';
        BOOST_TEST(to_r);

        T ret_val;
        auto from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val);
        if (!BOOST_TEST(from_r))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nRet val:" << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_fixed_format()
{
    constexpr double max_value = 1e10;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256];

        const auto val {dist(rng)};
        const T dec_val {val};

        auto to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val, chars_format::fixed);
        *to_r.ptr = '\0';
        BOOST_TEST(to_r);

        T ret_val;
        auto from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val, chars_format::fixed);
        if (!BOOST_TEST(from_r))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nRet val:" << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_hex_format()
{
    constexpr double max_value = 1e10;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256] {};

        const auto val {dist(rng)};
        const T dec_val {val};

        auto to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val, chars_format::hex);
        BOOST_TEST(to_r);

        T ret_val;
        auto from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val, chars_format::hex);
        if (!BOOST_TEST(from_r))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "  Value: " << dec_val
                      << "\n Buffer: " << buffer
                      << "\nRet val: " << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

#ifdef BOOST_DECIMAL_HAS_STD_CHARCONV

template <typename T>
void test_scientific_format_std()
{
    constexpr double max_value = 1e10;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256] {};

        const auto val {dist(rng)};
        const T dec_val {val};

        std::to_chars_result to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val, std::chars_format::scientific);
        BOOST_TEST(to_r.ec == std::errc());

        T ret_val;
        std::from_chars_result from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val, std::chars_format::scientific);
        if (!BOOST_TEST(from_r.ec == std::errc()))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "  Value: " << dec_val
                      << "\n Buffer: " << buffer
                      << "\nRet val: " << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_fixed_format_std()
{
    constexpr double max_value = 1e10;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256] {};

        const auto val {dist(rng)};
        const T dec_val {val};

        std::to_chars_result to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val, std::chars_format::fixed);
        BOOST_TEST(to_r.ec == std::errc());

        T ret_val;
        std::from_chars_result from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val, std::chars_format::fixed);
        if (!BOOST_TEST(from_r.ec == std::errc()))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "  Value: " << dec_val
                      << "\n Buffer: " << buffer
                      << "\nRet val: " << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_hex_format_std()
{
    constexpr double max_value = 1e10;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256] {};

        const auto val {dist(rng)};
        const T dec_val {val};

        std::to_chars_result to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val, std::chars_format::hex);
        BOOST_TEST(to_r.ec == std::errc());

        T ret_val;
        std::from_chars_result from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val, std::chars_format::hex);
        if (!BOOST_TEST(from_r.ec == std::errc()))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "  Value: " << dec_val
                      << "\n Buffer: " << buffer
                      << "\nRet val: " << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_general_format_std()
{
    constexpr double max_value = 1e10;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    for (std::size_t i {}; i < N; ++i)
    {
        char buffer[256] {};

        const auto val {dist(rng)};
        const T dec_val {val};

        std::to_chars_result to_r = to_chars(buffer, buffer + sizeof(buffer), dec_val, std::chars_format::general);
        BOOST_TEST(to_r.ec == std::errc());

        T ret_val;
        std::from_chars_result from_r = from_chars(buffer, buffer + std::strlen(buffer), ret_val, std::chars_format::general);
        if (!BOOST_TEST(from_r.ec == std::errc()))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<T>::digits10)
                      << "  Value: " << dec_val
                      << "\n Buffer: " << buffer
                      << "\nRet val: " << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

#endif

template <typename T>
void test_value(T val, chars_format fmt, int precision, const char* result)
{
    char buffer[256] {};
    auto r = to_chars(buffer, buffer + sizeof(buffer), val, fmt, precision);
    *r.ptr = '\0';
    BOOST_TEST(r);
    BOOST_TEST_CSTR_EQ(result, buffer);
}

template <typename T>
void test_precision()
{
    test_value(T{11, -1}, chars_format::scientific, 1, "1.1e+00");
    test_value(T{11, -1}, chars_format::fixed, 1, "1.1");

    test_value(T{11, -1}, chars_format::scientific, 2, "1.10e+00");
    test_value(T{11, -1}, chars_format::fixed, 2, "1.10");

    test_value(T{11, -1}, chars_format::scientific, 3, "1.100e+00");
    test_value(T{11, -1}, chars_format::fixed, 3, "1.100");

    test_value(T{11, -1}, chars_format::scientific, 4, "1.1000e+00");
    test_value(T{11, -1}, chars_format::fixed, 4, "1.1000");

    test_value(T{11, -1}, chars_format::scientific, 5, "1.10000e+00");
    test_value(T{11, -1}, chars_format::fixed, 5, "1.10000");

    test_value(T{11, -1}, chars_format::scientific, 6, "1.100000e+00");
    test_value(T{11, -1}, chars_format::fixed, 6, "1.100000");

    test_value(T{11, -1}, chars_format::scientific, 50, "1.10000000000000000000000000000000000000000000000000e+00");
    test_value(T{11, -1}, chars_format::fixed, 50, "1.10000000000000000000000000000000000000000000000000");

    test_value(T{11, -1}, chars_format::general, 50, "1.1");
}

template <typename T>
void test_buffer_overflow()
{
    constexpr double max_value = 1e10;
    std::uniform_real_distribution<double> dist(-max_value, max_value);

    const auto formats = {chars_format::general, chars_format::scientific, chars_format::fixed};

    for (const auto format : formats)
    {
        for (std::size_t i {}; i < 10; ++i)
        {
            char buffer[4];
            T test_value {dist(rng)};

            const auto r = to_chars(buffer, buffer + sizeof(buffer), test_value, format);
            BOOST_TEST(!r);
        }
    }
}

template <typename T>
void zero_test()
{
    std::uniform_real_distribution<double> dist;

    constexpr T val {0, 0};

    // General should always be the same
    for (int precision = 0; precision < 50; ++precision)
    {
        test_value(val * T{dist(rng)}, "0.0e+00", chars_format::general, precision);
    }

    test_value(val * T{dist(rng)}, "0e+00", chars_format::scientific, 0);
    test_value(val * T{dist(rng)}, "0.0e+00", chars_format::scientific, 1);
    test_value(val * T{dist(rng)}, "0.00e+00", chars_format::scientific, 2);
    test_value(val * T{dist(rng)}, "0.000e+00", chars_format::scientific, 3);
    test_value(val * T{dist(rng)}, "0.0000e+00", chars_format::scientific, 4);
    test_value(val * T{dist(rng)}, "0.00000e+00", chars_format::scientific, 5);
    test_value(val * T{dist(rng)}, "0.000000e+00", chars_format::scientific, 6);
    test_value(val * T{dist(rng)}, "0.0000000e+00", chars_format::scientific, 7);
    test_value(val * T{dist(rng)}, "0.00000000e+00", chars_format::scientific, 8);
    test_value(val * T{dist(rng)}, "0.000000000e+00", chars_format::scientific, 9);
    test_value(val * T{dist(rng)}, "0.0000000000e+00", chars_format::scientific, 10);
    test_value(val * T{dist(rng)}, "0.00000000000000000000000000000000000000000000000000e+00", chars_format::scientific, 50);

    test_value(val * T{dist(rng)}, "0p+00", chars_format::hex, 0);
    test_value(val * T{dist(rng)}, "0.0p+00", chars_format::hex, 1);
    test_value(val * T{dist(rng)}, "0.00p+00", chars_format::hex, 2);
    test_value(val * T{dist(rng)}, "0.000p+00", chars_format::hex, 3);
    test_value(val * T{dist(rng)}, "0.0000p+00", chars_format::hex, 4);
    test_value(val * T{dist(rng)}, "0.00000p+00", chars_format::hex, 5);
    test_value(val * T{dist(rng)}, "0.000000p+00", chars_format::hex, 6);
    test_value(val * T{dist(rng)}, "0.0000000p+00", chars_format::hex, 7);
    test_value(val * T{dist(rng)}, "0.00000000p+00", chars_format::hex, 8);
    test_value(val * T{dist(rng)}, "0.000000000p+00", chars_format::hex, 9);
    test_value(val * T{dist(rng)}, "0.0000000000p+00", chars_format::hex, 10);
    test_value(val * T{dist(rng)}, "0.00000000000000000000000000000000000000000000000000p+00", chars_format::hex, 50);

    test_value(val * T{dist(rng)}, "0", chars_format::fixed, 0);
    test_value(val * T{dist(rng)}, "0.0", chars_format::fixed, 1);
    test_value(val * T{dist(rng)}, "0.00", chars_format::fixed, 2);
    test_value(val * T{dist(rng)}, "0.000", chars_format::fixed, 3);
    test_value(val * T{dist(rng)}, "0.0000", chars_format::fixed, 4);
    test_value(val * T{dist(rng)}, "0.00000", chars_format::fixed, 5);
    test_value(val * T{dist(rng)}, "0.000000", chars_format::fixed, 6);
    test_value(val * T{dist(rng)}, "0.0000000", chars_format::fixed, 7);
    test_value(val * T{dist(rng)}, "0.00000000", chars_format::fixed, 8);
    test_value(val * T{dist(rng)}, "0.000000000", chars_format::fixed, 9);
    test_value(val * T{dist(rng)}, "0.0000000000", chars_format::fixed, 10);
    test_value(val * T{dist(rng)}, "0.00000000000000000000000000000000000000000000000000", chars_format::fixed, 50);
    
    test_value(val * T{dist(rng)}, "0", chars_format::fixed);
}

// See: https://github.com/cppalliance/decimal/issues/434
template <typename T>
void test_434_fixed()
{
    constexpr T test_zero_point_three {3, -1};

    test_value(test_zero_point_three, "0", chars_format::fixed, 0);
    test_value(test_zero_point_three, "0.3", chars_format::fixed, 1);
    test_value(test_zero_point_three, "0.30", chars_format::fixed, 2);
    test_value(test_zero_point_three, "0.300", chars_format::fixed, 3);
    test_value(test_zero_point_three, "0.3000", chars_format::fixed, 4);
    test_value(test_zero_point_three, "0.30000", chars_format::fixed, 5);
    test_value(test_zero_point_three, "0.300000", chars_format::fixed, 6);
    test_value(test_zero_point_three, "0.300000", chars_format::fixed, -1);
    test_value(test_zero_point_three, "0.3000000", chars_format::fixed, 7);
    test_value(test_zero_point_three, "0.30000000", chars_format::fixed, 8);
    test_value(test_zero_point_three, "0.300000000", chars_format::fixed, 9);
    test_value(test_zero_point_three, "0.3000000000", chars_format::fixed, 10);
    test_value(test_zero_point_three, "0.30000000000000000000000000000000000000000000000000", chars_format::fixed, 50);

    constexpr T test_one_and_quarter {125, -2};

    test_value(test_one_and_quarter, "1", chars_format::fixed, 0);
    test_value(test_one_and_quarter, "1.3", chars_format::fixed, 1);
    test_value(test_one_and_quarter, "1.25", chars_format::fixed, 2);
    test_value(test_one_and_quarter, "1.250", chars_format::fixed, 3);
    test_value(test_one_and_quarter, "1.2500", chars_format::fixed, 4);
    test_value(test_one_and_quarter, "1.25000", chars_format::fixed, 5);
    test_value(test_one_and_quarter, "1.250000", chars_format::fixed, 6);
    test_value(test_one_and_quarter, "1.250000", chars_format::fixed, -1);
    test_value(test_one_and_quarter, "1.2500000", chars_format::fixed, 7);
    test_value(test_one_and_quarter, "1.25000000", chars_format::fixed, 8);
    test_value(test_one_and_quarter, "1.250000000", chars_format::fixed, 9);
    test_value(test_one_and_quarter, "1.2500000000", chars_format::fixed, 10);
    test_value(test_one_and_quarter, "1.25000000000000000000000000000000000000000000000000", chars_format::fixed, 50);

    constexpr T tweleve_and_half {125, -1};

    test_value(tweleve_and_half, "13", chars_format::fixed, 0);
    test_value(tweleve_and_half, "12.5", chars_format::fixed, 1);
    test_value(tweleve_and_half, "12.50", chars_format::fixed, 2);
    test_value(tweleve_and_half, "12.500", chars_format::fixed, 3);
    test_value(tweleve_and_half, "12.5000", chars_format::fixed, 4);
    test_value(tweleve_and_half, "12.50000", chars_format::fixed, 5);
    test_value(tweleve_and_half, "12.500000", chars_format::fixed, 6);
    test_value(tweleve_and_half, "12.500000", chars_format::fixed, -1);
    test_value(tweleve_and_half, "12.5000000", chars_format::fixed, 7);
    test_value(tweleve_and_half, "12.50000000", chars_format::fixed, 8);
    test_value(tweleve_and_half, "12.500000000", chars_format::fixed, 9);
    test_value(tweleve_and_half, "12.5000000000", chars_format::fixed, 10);
    test_value(tweleve_and_half, "12.50000000000000000000000000000000000000000000000000", chars_format::fixed, 50);

    constexpr T one_e_minus_two {1, -2};

    test_value(one_e_minus_two, "0.010000", chars_format::fixed, -1);
    test_value(one_e_minus_two, "0", chars_format::fixed, 0);
    test_value(one_e_minus_two, "0.0", chars_format::fixed, 1);
    test_value(one_e_minus_two, "0.01", chars_format::fixed, 2);
    test_value(one_e_minus_two, "0.010", chars_format::fixed, 3);
    test_value(one_e_minus_two, "0.0100", chars_format::fixed, 4);
    test_value(one_e_minus_two, "0.01000", chars_format::fixed, 5);
    test_value(one_e_minus_two, "0.010000", chars_format::fixed, 6);
    test_value(one_e_minus_two, "0.010000", chars_format::fixed, -1);
    test_value(one_e_minus_two, "0.0100000", chars_format::fixed, 7);
    test_value(one_e_minus_two, "0.01000000", chars_format::fixed, 8);
    test_value(one_e_minus_two, "0.010000000", chars_format::fixed, 9);
    test_value(one_e_minus_two, "0.0100000000", chars_format::fixed, 10);
    test_value(one_e_minus_two, "0.01000000000000000000000000000000000000000000000000", chars_format::fixed, 50);

    constexpr T one_e_minus_three {1, -3};

    test_value(one_e_minus_three, "0.001000", chars_format::fixed, -1);
    test_value(one_e_minus_three, "0", chars_format::fixed, 0);
    test_value(one_e_minus_three, "0.0", chars_format::fixed, 1);
    test_value(one_e_minus_three, "0.00", chars_format::fixed, 2);
    test_value(one_e_minus_three, "0.001", chars_format::fixed, 3);
    test_value(one_e_minus_three, "0.0010", chars_format::fixed, 4);
    test_value(one_e_minus_three, "0.00100", chars_format::fixed, 5);
    test_value(one_e_minus_three, "0.001000", chars_format::fixed, 6);
    test_value(one_e_minus_three, "0.0010000", chars_format::fixed, 7);
    test_value(one_e_minus_three, "0.00100000", chars_format::fixed, 8);
    test_value(one_e_minus_three, "0.001000000", chars_format::fixed, 9);
    test_value(one_e_minus_three, "0.0010000000", chars_format::fixed, 10);
    test_value(one_e_minus_three, "0.00100000000000000000000000000000000000000000000000", chars_format::fixed, 50);

    constexpr T ten {1, 1};

    test_value(ten, "10.000000", chars_format::fixed, -1);
    test_value(ten, "10", chars_format::fixed, 0);
    test_value(ten, "10.0", chars_format::fixed, 1);
    test_value(ten, "10.00", chars_format::fixed, 2);
    test_value(ten, "10.000", chars_format::fixed, 3);
    test_value(ten, "10.0000", chars_format::fixed, 4);
    test_value(ten, "10.00000", chars_format::fixed, 5);
    test_value(ten, "10.000000", chars_format::fixed, 6);
    test_value(ten, "10.0000000", chars_format::fixed, 7);
    test_value(ten, "10.00000000", chars_format::fixed, 8);
    test_value(ten, "10.000000000", chars_format::fixed, 9);
    test_value(ten, "10.0000000000", chars_format::fixed, 10);
    test_value(ten, "10.00000000000000000000000000000000000000000000000000", chars_format::fixed, 50);

    constexpr T twelve_and_half {125, -1};

    test_value(twelve_and_half, "12.500000", chars_format::fixed, -1);
    test_value(twelve_and_half, "13", chars_format::fixed, 0);
    test_value(twelve_and_half, "12.5", chars_format::fixed, 1);
    test_value(twelve_and_half, "12.50", chars_format::fixed, 2);
    test_value(twelve_and_half, "12.500", chars_format::fixed, 3);
    test_value(twelve_and_half, "12.5000", chars_format::fixed, 4);
    test_value(twelve_and_half, "12.50000", chars_format::fixed, 5);
    test_value(twelve_and_half, "12.500000", chars_format::fixed, 6);
    test_value(twelve_and_half, "12.5000000", chars_format::fixed, 7);
    test_value(twelve_and_half, "12.50000000", chars_format::fixed, 8);
    test_value(twelve_and_half, "12.500000000", chars_format::fixed, 9);
    test_value(twelve_and_half, "12.5000000000", chars_format::fixed, 10);
    test_value(twelve_and_half, "12.50000000000000000000000000000000000000000000000000", chars_format::fixed, 50);
}

template <typename T>
void test_434_scientific()
{
    constexpr T test_zero_point_three {3, -1};

    test_value(test_zero_point_three, "3e-01", chars_format::scientific, 0);
    test_value(test_zero_point_three, "3.0e-01", chars_format::scientific, 1);
    test_value(test_zero_point_three, "3.00e-01", chars_format::scientific, 2);
    test_value(test_zero_point_three, "3.000e-01", chars_format::scientific, 3);
    test_value(test_zero_point_three, "3.0000e-01", chars_format::scientific, 4);
    test_value(test_zero_point_three, "3.00000e-01", chars_format::scientific, 5);
    test_value(test_zero_point_three, "3.000000e-01", chars_format::scientific, 6);
    test_value(test_zero_point_three, "3.000000e-01", chars_format::scientific, -1);
    test_value(test_zero_point_three, "3.0000000e-01", chars_format::scientific, 7);
    test_value(test_zero_point_three, "3.00000000e-01", chars_format::scientific, 8);
    test_value(test_zero_point_three, "3.000000000e-01", chars_format::scientific, 9);
    test_value(test_zero_point_three, "3.0000000000e-01", chars_format::scientific, 10);
    test_value(test_zero_point_three, "3.00000000000000000000000000000000000000000000000000e-01", chars_format::scientific, 50);

    constexpr T test_one_and_quarter {125, -2};

    test_value(test_one_and_quarter, "1e+00", chars_format::scientific, 0);
    test_value(test_one_and_quarter, "1.3e+00", chars_format::scientific, 1);
    test_value(test_one_and_quarter, "1.25e+00", chars_format::scientific, 2);
    test_value(test_one_and_quarter, "1.250e+00", chars_format::scientific, 3);
    test_value(test_one_and_quarter, "1.2500e+00", chars_format::scientific, 4);
    test_value(test_one_and_quarter, "1.25000e+00", chars_format::scientific, 5);
    test_value(test_one_and_quarter, "1.250000e+00", chars_format::scientific, 6);
    test_value(test_one_and_quarter, "1.250000e+00", chars_format::scientific, -1);
    test_value(test_one_and_quarter, "1.2500000e+00", chars_format::scientific, 7);
    test_value(test_one_and_quarter, "1.25000000e+00", chars_format::scientific, 8);
    test_value(test_one_and_quarter, "1.250000000e+00", chars_format::scientific, 9);
    test_value(test_one_and_quarter, "1.2500000000e+00", chars_format::scientific, 10);
    test_value(test_one_and_quarter, "1.25000000000000000000000000000000000000000000000000e+00", chars_format::scientific, 50);

    constexpr T tweleve_and_half {125, -1};

    test_value(tweleve_and_half, "1e+01", chars_format::scientific, 0);
    test_value(tweleve_and_half, "1.3e+01", chars_format::scientific, 1);
    test_value(tweleve_and_half, "1.25e+01", chars_format::scientific, 2);
    test_value(tweleve_and_half, "1.250e+01", chars_format::scientific, 3);
    test_value(tweleve_and_half, "1.2500e+01", chars_format::scientific, 4);
    test_value(tweleve_and_half, "1.25000e+01", chars_format::scientific, 5);
    test_value(tweleve_and_half, "1.250000e+01", chars_format::scientific, 6);
    test_value(tweleve_and_half, "1.250000e+01", chars_format::scientific, -1);
    test_value(tweleve_and_half, "1.2500000e+01", chars_format::scientific, 7);
    test_value(tweleve_and_half, "1.25000000e+01", chars_format::scientific, 8);
    test_value(tweleve_and_half, "1.250000000e+01", chars_format::scientific, 9);
    test_value(tweleve_and_half, "1.2500000000e+01", chars_format::scientific, 10);
    test_value(tweleve_and_half, "1.25000000000000000000000000000000000000000000000000e+01", chars_format::scientific, 50);

    constexpr T one_e_minus_two {1, -2};

    test_value(one_e_minus_two, "1e-02", chars_format::scientific, 0);
    test_value(one_e_minus_two, "1.0e-02", chars_format::scientific, 1);
    test_value(one_e_minus_two, "1.00e-02", chars_format::scientific, 2);
    test_value(one_e_minus_two, "1.000e-02", chars_format::scientific, 3);
    test_value(one_e_minus_two, "1.0000e-02", chars_format::scientific, 4);
    test_value(one_e_minus_two, "1.00000e-02", chars_format::scientific, 5);
    test_value(one_e_minus_two, "1.000000e-02", chars_format::scientific, 6);
    test_value(one_e_minus_two, "1.000000e-02", chars_format::scientific, -1);
    test_value(one_e_minus_two, "1.0000000e-02", chars_format::scientific, 7);
    test_value(one_e_minus_two, "1.00000000e-02", chars_format::scientific, 8);
    test_value(one_e_minus_two, "1.000000000e-02", chars_format::scientific, 9);
    test_value(one_e_minus_two, "1.00000000000000000000000000000000000000000000000000e-02", chars_format::scientific, 50);
}

template <typename T>
void test_434_hex()
{
    constexpr T one {1, 0};

    test_value(one, "1p+00", chars_format::hex, 0);
    test_value(one, "1.0p+00", chars_format::hex, 1);
    test_value(one, "1.00p+00", chars_format::hex, 2);
    test_value(one, "1.000p+00", chars_format::hex, 3);
    test_value(one, "1.0000p+00", chars_format::hex, 4);
    test_value(one, "1.00000p+00", chars_format::hex, 5);
    test_value(one, "1.000000p+00", chars_format::hex, 6);
    test_value(one, "1.0000000p+00", chars_format::hex, 7);
    test_value(one, "1.00000000p+00", chars_format::hex, 8);
    test_value(one, "1.000000000p+00", chars_format::hex, 9);
    test_value(one, "1.0000000000p+00", chars_format::hex, 10);
    test_value(one, "1.00000000000000000000000000000000000000000000000000p+00", chars_format::hex, 50);

    constexpr T test_zero_point_three {3, -1};

    test_value(test_zero_point_three, "3p-01", chars_format::hex, 0);
    test_value(test_zero_point_three, "3.0p-01", chars_format::hex, 1);
    test_value(test_zero_point_three, "3.00p-01", chars_format::hex, 2);
    test_value(test_zero_point_three, "3.000p-01", chars_format::hex, 3);
    test_value(test_zero_point_three, "3.0000p-01", chars_format::hex, 4);
    test_value(test_zero_point_three, "3.00000p-01", chars_format::hex, 5);
    test_value(test_zero_point_three, "3.000000p-01", chars_format::hex, 6);
    test_value(test_zero_point_three, "3.0000000p-01", chars_format::hex, 7);
    test_value(test_zero_point_three, "3.00000000p-01", chars_format::hex, 8);
    test_value(test_zero_point_three, "3.000000000p-01", chars_format::hex, 9);
    test_value(test_zero_point_three, "3.0000000000p-01", chars_format::hex, 10);
    test_value(test_zero_point_three, "3.00000000000000000000000000000000000000000000000000p-01", chars_format::hex, 50);

    constexpr T test_one_and_quarter {125, -2};

    test_value(test_one_and_quarter, "8p-01", chars_format::hex, 0);
    test_value(test_one_and_quarter, "7.dp-01", chars_format::hex, 1);
    test_value(test_one_and_quarter, "7.d0p-01", chars_format::hex, 2);
    test_value(test_one_and_quarter, "7.d00p-01", chars_format::hex, 3);
    test_value(test_one_and_quarter, "7.d000p-01", chars_format::hex, 4);
    test_value(test_one_and_quarter, "7.d0000p-01", chars_format::hex, 5);
    test_value(test_one_and_quarter, "7.d00000p-01", chars_format::hex, 6);
    test_value(test_one_and_quarter, "7.d000000p-01", chars_format::hex, 7);
    test_value(test_one_and_quarter, "7.d0000000p-01", chars_format::hex, 8);
    test_value(test_one_and_quarter, "7.d00000000p-01", chars_format::hex, 9);
    test_value(test_one_and_quarter, "7.d000000000p-01", chars_format::hex, 10);
    test_value(test_one_and_quarter, "7.d0000000000000000000000000000000000000000000000000p-01", chars_format::hex, 50);
}

template <typename T>
void test_777()
{
    constexpr T value1 = T {21, 6, true};
    constexpr T value2 = T {211, 6, true};
    constexpr T value3 = T {2111, 6, true};

    test_value(value1, "-21000000", chars_format::fixed, 0);
    test_value(value2, "-211000000", chars_format::fixed, 0);
    test_value(value3, "-2111000000", chars_format::fixed, 0);
}

template <typename T>
void test_more_powers_10()
{
    test_value(T{1, -6}, "0.000001", chars_format::fixed);
    test_value(T{1, -5}, "0.00001", chars_format::fixed);
    test_value(T{1, -4}, "0.0001", chars_format::fixed);
    test_value(T{1, -3}, "0.001", chars_format::fixed);
    test_value(T{1, -2}, "0.01", chars_format::fixed);
    test_value(T{1, -1}, "0.1", chars_format::fixed);
    test_value(T{1, 0}, "1", chars_format::fixed);
    test_value(T{1, 1}, "10", chars_format::fixed);
    test_value(T{1, 2}, "100", chars_format::fixed);
    test_value(T{1, 3}, "1000", chars_format::fixed);
    test_value(T{1, 4}, "10000", chars_format::fixed);
    test_value(T{1, 5}, "100000", chars_format::fixed);
    test_value(T{1, 6}, "1000000", chars_format::fixed);
    test_value(T{1, 7}, "10000000", chars_format::fixed);
}

int main()
{
    test_non_finite_values<decimal32>();
    test_non_finite_values<decimal64>();

    test_small_values<decimal32>();
    test_small_values<decimal64>();

    test_large_values<decimal32>();
    test_large_values<decimal64>();

    test_fixed_format<decimal32>();
    test_fixed_format<decimal64>();

    test_precision<decimal32>();
    test_precision<decimal64>();

    test_buffer_overflow<decimal32>();
    test_buffer_overflow<decimal64>();

    zero_test<decimal32>();
    zero_test<decimal64>();

    test_434_fixed<decimal32>();
    test_434_fixed<decimal64>();

    test_434_scientific<decimal32>();
    test_434_scientific<decimal64>();

    test_hex_format<decimal32>();
    test_hex_format<decimal64>();

    test_434_hex<decimal32>();
    test_434_hex<decimal64>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test_non_finite_values<decimal128>();
    test_small_values<decimal128>();
    test_large_values<decimal128>();
    test_fixed_format<decimal128>();
    test_precision<decimal128>();
    test_buffer_overflow<decimal128>();
    zero_test<decimal128>();
    test_434_fixed<decimal128>();
    test_434_scientific<decimal128>();
    test_hex_format<decimal128>();
    test_434_hex<decimal128>();
    #endif

    test_non_finite_values<decimal32_fast>();
    test_small_values<decimal32_fast>();
    test_large_values<decimal32_fast>();
    test_fixed_format<decimal32_fast>();
    test_precision<decimal32_fast>();
    test_buffer_overflow<decimal32_fast>();
    zero_test<decimal32_fast>();
    test_434_fixed<decimal32_fast>();
    test_434_scientific<decimal32_fast>();
    test_hex_format<decimal32_fast>();
    test_434_hex<decimal32_fast>();

    test_non_finite_values<decimal64_fast>();
    test_small_values<decimal64_fast>();
    test_large_values<decimal64_fast>();
    test_fixed_format<decimal64_fast>();
    test_precision<decimal64_fast>();
    test_buffer_overflow<decimal64_fast>();
    zero_test<decimal64_fast>();
    test_434_fixed<decimal64_fast>();
    test_434_scientific<decimal64_fast>();
    test_hex_format<decimal64_fast>();
    test_434_hex<decimal64_fast>();

    // Bugfixes
    test_value(decimal64{2657844750}, "2657844750", chars_format::general);

    // See: https://github.com/cppalliance/decimal/issues/470
    test_value(decimal32{504.29034} / decimal32{-727.45465}, "-0.693226", chars_format::general, 6);
    test_value(decimal32{504.29034} / decimal32{-727.45465}, "-6.932257e-01", chars_format::scientific, 6);

    // Value found from fuzzing
    for (int precision = -1; precision < 10; ++precision)
    {
        test_error_value<decimal64>("e1000a00000000000000000000p06", chars_format::hex, precision);
    }

    #ifdef BOOST_DECIMAL_HAS_STD_CHARCONV
    test_scientific_format_std<decimal32>();
    test_scientific_format_std<decimal64>();

    test_fixed_format_std<decimal32>();
    test_fixed_format_std<decimal64>();

    test_hex_format_std<decimal32>();
    test_hex_format_std<decimal64>();

    test_general_format_std<decimal32>();
    test_general_format_std<decimal64>();
    #endif

    test_777<decimal32>();
    test_777<decimal64>();
    test_777<decimal128>();
    test_777<decimal32_fast>();
    test_777<decimal64_fast>();
    test_777<decimal128_fast>();

    test_more_powers_10<decimal32>();
    test_more_powers_10<decimal64>();
    test_more_powers_10<decimal128>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
