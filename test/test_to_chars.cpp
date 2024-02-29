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
            std::cerr << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nRet val:" << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_large_values()
{
    constexpr double max_value = std::is_same<decimal32, T>::value ? 1e80 : 1e200;
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
            std::cerr << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << "Value: " << dec_val
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
            std::cerr << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nError: " << static_cast<int>(from_r.ec) << std::endl;

            continue;
            // LCOV_EXCL_STOP
        }

        if (!BOOST_TEST_EQ(dec_val, ret_val))
        {
            // LCOV_EXCL_START
            std::cerr << "Value: " << dec_val
                      << "\nBuffer: " << buffer
                      << "\nRet val:" << ret_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
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

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test_non_finite_values<decimal128>();
    test_small_values<decimal128>();
    test_large_values<decimal128>();
    test_fixed_format<decimal128>();
    #endif

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
