// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include <cwchar>

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

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(1024U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(1024U >> 4U); // Number of trials
#endif

template <typename T>
void roundtrip_strtod()
{
    std::mt19937_64 rng(42);

    std::uniform_int_distribution<std::int64_t> sig(1'000'000, 9'999'999);
    std::uniform_int_distribution<std::int32_t> exp(std::numeric_limits<T>::min_exponent10 + 19,
                                                    std::numeric_limits<T>::max_exponent10 - 19);

    constexpr auto max_iter {std::is_same<T, decimal128>::value ? N / 4 : N};
    for (std::size_t i {}; i < max_iter; ++i)
    {
        const T val {sig(rng), exp(rng)};
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<T>::digits10);
        ss << val;
        char* endptr {};

        const T return_val {boost::decimal::strtod<T>(ss.str().c_str(), &endptr)};
        const auto len {std::strlen(ss.str().c_str())};
        const auto dist {endptr - ss.str().c_str()};

        if (!BOOST_TEST_EQ(val, return_val) && BOOST_TEST_EQ(len, dist))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val
                      << "\nVal 2: " << return_val
                      << "\nStrlen: " << len
                      << "\n  Dist: " << dist << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void roundtrip_wcstrtod()
{
    std::mt19937_64 rng(42);

    std::uniform_int_distribution<std::int64_t> sig(1'000'000, 9'999'999);
    std::uniform_int_distribution<std::int32_t> exp(std::numeric_limits<T>::min_exponent10 + 19,
                                                    std::numeric_limits<T>::max_exponent10 - 19);

    constexpr auto max_iter {std::is_same<T, decimal128>::value ? N / 4 : N};
    for (std::size_t i {}; i < max_iter; ++i)
    {
        const T val {sig(rng), exp(rng)};
        std::wstringstream ss;
        ss << std::setprecision(std::numeric_limits<T>::digits10);
        ss << val;
        wchar_t* endptr {};

        const T return_val {boost::decimal::wcstod<T>(ss.str().c_str(), &endptr)};
        const auto len {ss.str().size()};
        const auto dist {endptr - ss.str().c_str()};

        if (!BOOST_TEST_EQ(val, return_val) && BOOST_TEST_EQ(len, dist))
        {
            // LCOV_EXCL_START
            std::cerr << std::scientific
                      << std::setprecision(std::numeric_limits<T>::digits10)
                      << "Val 1: " << val
                      << "\nVal 2: " << return_val
                      << "\nStrlen: " << len
                      << "\n  Dist: " << dist << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_strtod_edges()
{
    errno = 0;
    BOOST_TEST(isnan(boost::decimal::strtod<T>(nullptr, nullptr))) && BOOST_TEST_EQ(errno, EINVAL);

    errno = 0;
    BOOST_TEST(isnan(boost::decimal::wcstod<T>(nullptr, nullptr))) && BOOST_TEST_EQ(errno, EINVAL);

    errno = 0;
    const char* snan_str = "nan(snan)";
    BOOST_TEST(isnan(boost::decimal::strtod<T>(snan_str, nullptr))) && BOOST_TEST_EQ(errno, 0);

    errno = 0;
    const char* qnan_str = "nan";
    BOOST_TEST(isnan(boost::decimal::strtod<T>(qnan_str, nullptr))) && BOOST_TEST_EQ(errno, 0);

    errno = 0;
    const char* inf_str = "inf";
    BOOST_TEST(isinf(boost::decimal::strtod<T>(inf_str, nullptr))) && BOOST_TEST_EQ(errno, 0);

    errno = 0;
    const char* junk_str = "junk";
    BOOST_TEST(isnan(boost::decimal::strtod<T>(junk_str, nullptr))) && BOOST_TEST_EQ(errno, EINVAL);
}

template <typename T>
void test_locales()
{
    const char buffer[] = "1,1897e+02";
    const auto valdiation_value = static_cast<T>(1.1897e+02);

    try
    {
        #ifdef BOOST_MSVC
        std::locale::global(std::locale("German"));
        #else
        std::locale::global(std::locale("de_DE.UTF-8"));
        #endif
    }
    // LCOV_EXCL_START
    catch (...)
    {
        std::cerr << "Locale not installed. Skipping test." << std::endl;
        return;
    }
    // LCOV_EXCL_STOP

    char* enddptr;
    const auto val = boost::decimal::strtod<T>(buffer, &enddptr);
    BOOST_TEST_EQ(valdiation_value, val);
}

int main()
{
    roundtrip_strtod<decimal32>();
    roundtrip_wcstrtod<decimal32>();
    test_strtod_edges<decimal32>();

    roundtrip_strtod<decimal64>();
    roundtrip_wcstrtod<decimal64>();
    test_strtod_edges<decimal64>();

    roundtrip_strtod<decimal128>();
    roundtrip_wcstrtod<decimal128>();
    test_strtod_edges<decimal128>();

    test_locales<decimal32>();
    test_locales<decimal64>();
    test_locales<decimal128>();

    return boost::report_errors();
}
