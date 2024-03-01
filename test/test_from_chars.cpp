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
void test_from_chars_scientific()
{
    std::uniform_real_distribution<float> dist(1e-10F, 1e10F);

    constexpr auto max_iter {std::is_same<T, decimal128>::value ? N / 4 : N};

    for (std::size_t i {}; i < max_iter; ++i)
    {
        char buffer[256] {};
        const auto val {dist(rng)};
        auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), val, boost::charconv::chars_format::scientific);

        if (!r)
        {
            continue; // LCOV_EXCL_LINE
        }

        *r.ptr = '\0';

        T return_value;
        const auto r_dec = from_chars(buffer, buffer + std::strlen(buffer), return_value, chars_format::scientific);
        const auto ret_value_float = static_cast<float>(return_value);
        const auto float_distance = std::abs(boost::math::float_distance(ret_value_float, val));

        if (!(BOOST_TEST(float_distance <= 10) && BOOST_TEST(r_dec)))
        {
            // LCOV_EXCL_START
            std::cerr << "     Value: " << val
                      << "\n    Buffer: " << buffer
                      << "\n   Ret Val: " << return_value
                      << "\nFloat dist: " << float_distance << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_from_chars_fixed()
{
    std::uniform_real_distribution<float> dist(1e-10F, 1e10F);

    constexpr auto max_iter {std::is_same<T, decimal128>::value ? N / 4 : N};

    for (std::size_t i {}; i < max_iter; ++i)
    {
        char buffer[256] {};
        const auto val {dist(rng)};
        auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), val, boost::charconv::chars_format::fixed);

        if (!r)
        {
            continue; // LCOV_EXCL_LINE
        }

        *r.ptr = '\0';

        T return_value;
        const auto r_dec = from_chars(buffer, buffer + std::strlen(buffer), return_value, chars_format::fixed);
        const auto ret_value_float = static_cast<float>(return_value);
        const auto float_distance = std::abs(boost::math::float_distance(ret_value_float, val));

        if (!(BOOST_TEST(float_distance <= 10) && BOOST_TEST(r_dec)))
        {
            // LCOV_EXCL_START
            std::cerr << "     Value: " << val
                      << "\n    Buffer: " << buffer
                      << "\n   Ret Val: " << return_value
                      << "\nFloat dist: " << float_distance << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_from_chars_general()
{
    std::uniform_real_distribution<float> dist(1e-10F, 1e10F);

    constexpr auto max_iter {std::is_same<T, decimal128>::value ? N / 4 : N};

    for (std::size_t i {}; i < max_iter; ++i)
    {
        char buffer[256] {};
        const auto val {dist(rng)};
        auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), val, boost::charconv::chars_format::general);

        if (!r)
        {
            continue; // LCOV_EXCL_LINE
        }

        *r.ptr = '\0';

        T return_value;
        const auto r_dec = from_chars(buffer, buffer + std::strlen(buffer), return_value, chars_format::general);
        const auto ret_value_float = static_cast<float>(return_value);
        const auto float_distance = std::abs(boost::math::float_distance(ret_value_float, val));

        if (!(BOOST_TEST(float_distance <= 10) && BOOST_TEST(r_dec)))
        {
            // LCOV_EXCL_START
            std::cerr << "     Value: " << val
                      << "\n    Buffer: " << buffer
                      << "\n   Ret Val: " << return_value
                      << "\nFloat dist: " << float_distance << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_non_finite_values()
{
    T val;
    BOOST_TEST(from_chars(nullptr, nullptr, val).ec == std::errc::invalid_argument);

    const char* snan_str = "nan(snan)";
    auto r = from_chars(snan_str, snan_str + std::strlen(snan_str), val);
    BOOST_TEST(isnan(val));

    const char* qnan_str = "nan";
    val = 0;
    r = from_chars(qnan_str, qnan_str + std::strlen(qnan_str), val);
    BOOST_TEST(r);
    BOOST_TEST(isnan(val));

    const char* inf_str = "inf";
    val = 0;
    r = from_chars(inf_str, inf_str + std::strlen(inf_str), val);
    BOOST_TEST(r.ec == std::errc::result_out_of_range);
    BOOST_TEST(isinf(val));
}

int main()
{
    test_from_chars_scientific<decimal32>();
    test_from_chars_scientific<decimal64>();

    test_from_chars_fixed<decimal32>();
    test_from_chars_fixed<decimal64>();

    test_from_chars_general<decimal32>();
    test_from_chars_general<decimal64>();

    test_non_finite_values<decimal32>();
    test_non_finite_values<decimal64>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test_from_chars_scientific<decimal128>();
    test_from_chars_fixed<decimal128>();
    test_from_chars_general<decimal128>();
    test_non_finite_values<decimal128>();
    #endif

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
