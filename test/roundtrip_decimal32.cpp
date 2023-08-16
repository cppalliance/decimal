// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>
#include <random>
#include <cerrno>

using namespace boost::decimal;

static constexpr std::size_t N = 1024; // Number of trials

template <typename T>
void test_conversion_to_integer()
{
    errno = 0;
    constexpr decimal32 neg_one(-1, 0);
    constexpr decimal32 one(1, 0);
    constexpr decimal32 zero(0, 0);
    constexpr decimal32 half(5, -1);
    BOOST_TEST_EQ(static_cast<T>(one), static_cast<T>(1)) && BOOST_TEST_EQ(errno, 0);
    BOOST_TEST_EQ(static_cast<T>(one + one), static_cast<T>(2)) && BOOST_TEST_EQ(errno, 0);
    BOOST_TEST_EQ(static_cast<T>(zero), static_cast<T>(0)) && BOOST_TEST_EQ(errno, 0);

    BOOST_IF_CONSTEXPR (std::is_signed<T>::value)
    {
        BOOST_TEST_EQ(static_cast<T>(neg_one), static_cast<T>(-1)) && BOOST_TEST_EQ(errno, 0);
    }
    else
    {
        // Bad conversion so we use zero
        BOOST_TEST_EQ(static_cast<T>(neg_one), static_cast<T>(0)) && BOOST_TEST_EQ(errno, ERANGE);
    }

    errno = 0;
    BOOST_TEST_EQ(static_cast<T>(std::numeric_limits<decimal32>::infinity()), static_cast<T>(0)) && BOOST_TEST_EQ(errno, ERANGE);

    errno = 0;
    BOOST_TEST_EQ(static_cast<T>(-std::numeric_limits<decimal32>::infinity()), static_cast<T>(0)) && BOOST_TEST_EQ(errno, ERANGE);

    errno = 0;
    BOOST_TEST_EQ(static_cast<T>(std::numeric_limits<decimal32>::quiet_NaN()), static_cast<T>(0)) && BOOST_TEST_EQ(errno, EINVAL);

    errno = 0;
    BOOST_TEST_EQ(static_cast<T>(std::numeric_limits<decimal32>::signaling_NaN()), static_cast<T>(0)) && BOOST_TEST_EQ(errno, EINVAL);

    errno = 0;
    BOOST_TEST_EQ(static_cast<T>(half), static_cast<T>(0)) && BOOST_TEST_EQ(errno, 0);

    constexpr decimal32 one_e_8(1, 8);
    BOOST_TEST_EQ(static_cast<T>(one_e_8), static_cast<T>(100'000'000)) && BOOST_TEST_EQ(errno, 0);

    constexpr decimal32 one_e_8_2(1'000'000, 2);
    BOOST_TEST_EQ(static_cast<T>(one_e_8_2), static_cast<T>(100'000'000)) && BOOST_TEST_EQ(errno, 0);
}

template <typename T>
void test_roundtrip_conversion()
{
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<T> dist(0, detail::max_significand);

    for (std::size_t i = 0; i < N; ++i)
    {
        const T val = dist(rng);
        const decimal32 initial_decimal(val);
        const T return_val (initial_decimal);
        const decimal32 return_decimal(return_val);

        BOOST_TEST_EQ(val, return_val);
        BOOST_TEST_EQ(initial_decimal, return_decimal);
    }

    // These will have loss of precision for the integer,
    // but should roundtrip for the decimal part
    std::uniform_int_distribution<T> big_dist((std::numeric_limits<T>::min)(), (std::numeric_limits<T>::max)());

    for (std::size_t i = 0; i < N; ++i)
    {
        const T val = dist(rng);
        const decimal32 initial_decimal(val);
        const T return_val (initial_decimal);
        const decimal32 return_decimal(return_val);

        BOOST_TEST_EQ(initial_decimal, return_decimal);
    }
}

int main()
{
    test_conversion_to_integer<int>();
    test_conversion_to_integer<unsigned>();
    test_conversion_to_integer<long>();
    test_conversion_to_integer<unsigned long>();
    test_conversion_to_integer<long long>();
    test_conversion_to_integer<unsigned long long>();

    test_roundtrip_conversion<int>();
    test_roundtrip_conversion<unsigned>();
    test_roundtrip_conversion<long>();
    test_roundtrip_conversion<unsigned long>();
    test_roundtrip_conversion<long long>();
    test_roundtrip_conversion<unsigned long long>();

    return boost::report_errors();
}
