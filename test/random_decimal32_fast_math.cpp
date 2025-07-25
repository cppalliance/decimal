// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal_fast32_t.hpp>
#include <boost/decimal/iostream.hpp>
#include <iostream>
#include <random>
#include <limits>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(1024U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(1024U >> 4U); // Number of trials
#endif

// NOLINTNEXTLINE : Seed with a constant for repeatability
static std::mt19937_64 rng(42); // NOSONAR : Global rng is not const

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4146)
#endif

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif


template <typename T>
void random_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const decimal_fast32_t dec2 {val2};

        const decimal_fast32_t res = dec1 + dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() + decimal_fast32_t{0,0}));
    BOOST_TEST(isinf(decimal_fast32_t{0,0} + std::numeric_limits<decimal_fast32_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() + decimal_fast32_t{0,0}));
    BOOST_TEST(isnan(decimal_fast32_t{0,0} + std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
}

template <typename T>
void random_mixed_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal_fast32_t(val2))};

        const decimal_fast32_t res = dec1 + trunc_val_2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << trunc_val_2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() + dist(rng)));
    BOOST_TEST(isinf(dist(rng) + std::numeric_limits<decimal_fast32_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() + dist(rng)));
    BOOST_TEST(isnan(dist(rng) + std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
}

template <typename T>
void random_converted_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        // Convert these to and from to ensure rounding
        const T val1 {static_cast<T>(decimal_fast32_t(dist(rng)))};
        const T val2 {static_cast<T>(decimal_fast32_t(dist(rng)))};

        const decimal_fast32_t dec1 {val1};
        const decimal_fast32_t dec2 {val2};

        const decimal_fast32_t res {dec1 + dec2};
        const decimal_fast32_t comp_val {val1 + val2};

        if (!BOOST_TEST_EQ(res, comp_val))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << comp_val << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void random_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const decimal_fast32_t dec2 {val2};

        const decimal_fast32_t res = dec1 - dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() - decimal_fast32_t{0,0}));
    BOOST_TEST(isinf(decimal_fast32_t{0,0} - std::numeric_limits<decimal_fast32_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() - decimal_fast32_t{0,0}));
    BOOST_TEST(isnan(decimal_fast32_t{0,0} - std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
}

template <typename T>
void random_mixed_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal_fast32_t(val2))};

        const decimal_fast32_t res = dec1 - trunc_val_2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << trunc_val_2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const T trunc_val_1 {static_cast<T>(decimal_fast32_t(val1))};
        const decimal_fast32_t dec2 {val2};

        const decimal_fast32_t res = trunc_val_1 - dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << trunc_val_1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() - dist(rng)));
    BOOST_TEST(isinf(dist(rng) - std::numeric_limits<decimal_fast32_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() - dist(rng)));
    BOOST_TEST(isnan(dist(rng) - std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
}

template <typename T>
void random_multiplication(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const decimal_fast32_t dec2 {val2};

        const decimal_fast32_t res {dec1 * dec2};
        const decimal_fast32_t res_int {val1 * val2};

        if (val1 * val2 == 0)
        {
            // Integers don't have signed 0 but decimal does
            continue;
        }

        if (!BOOST_TEST_EQ(res, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() * decimal_fast32_t(dist(rng))));
    BOOST_TEST(isinf(decimal_fast32_t(dist(rng)) * std::numeric_limits<decimal_fast32_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() * decimal_fast32_t(dist(rng))));
    BOOST_TEST(isnan(decimal_fast32_t(dist(rng)) * std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
}

template <typename T>
void random_spot_multiplication(T val1, T val2)
{
    const decimal_fast32_t dec1 {val1};
    const decimal_fast32_t dec2 {val2};

    const decimal_fast32_t res {dec1 * dec2};
    const decimal_fast32_t res_int {val1 * val2};

    if (!BOOST_TEST_EQ(res, res_int))
    {
        // LCOV_EXCL_START
        std::cerr << "Val 1: " << val1
                  << "\nDec 1: " << dec1
                  << "\nVal 2: " << val2
                  << "\nDec 2: " << dec2
                  << "\nDec res: " << res
                  << "\nInt res: " << val1 * val2 << std::endl;
        // LCOV_EXCL_STOP
    }
}

template <typename T>
void random_mixed_multiplication(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const T dec2 {static_cast<T>(decimal_fast32_t(val2))};

        const decimal_fast32_t res {dec1 * dec2};
        const decimal_fast32_t res_int {val1 * val2};

        if (val1 * val2 == 0)
        {
            // Integers don't have signed 0 but decimal does
            continue;
        }

        if (!BOOST_TEST_EQ(res, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() * dist(rng)));
    BOOST_TEST(isinf(dist(rng) * std::numeric_limits<decimal_fast32_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() * dist(rng)));
    BOOST_TEST(isnan(dist(rng) * std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
}

template <typename T>
void random_division(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const decimal_fast32_t dec2 {val2};

        const decimal_fast32_t res {dec1 / dec2};
        const decimal_fast32_t res_int {static_cast<float>(val1) / static_cast<float>(val2)};

        if (isinf(res) && isinf(res_int))
        {
        }
        else if (!BOOST_TEST(abs(res - res_int) < decimal_fast32_t(1, -2)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << static_cast<float>(val1) / static_cast<float>(val2) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() / decimal_fast32_t(dist(rng))));
    BOOST_TEST(!isinf(decimal_fast32_t(dist(rng)) / std::numeric_limits<decimal_fast32_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() / decimal_fast32_t(dist(rng))));
    BOOST_TEST(isnan(decimal_fast32_t(dist(rng)) / std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
    BOOST_TEST(isinf(decimal_fast32_t(dist(rng)) / decimal_fast32_t(0)));
}

template <typename T>
void random_mixed_division(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal_fast32_t dec1 {val1};
        const T dec2 {static_cast<T>(decimal_fast32_t(val2))};

        const decimal_fast32_t res {dec1 / dec2};
        const decimal_fast32_t res_int {static_cast<float>(val1) / static_cast<float>(val2)};

        if (isinf(res) && isinf(res_int))
        {
        }
        else if (!BOOST_TEST(abs(res - res_int) < decimal_fast32_t(1, -3)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << static_cast<float>(val1) / static_cast<float>(val2) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const T dec1 {static_cast<T>(decimal_fast32_t(val1))};
        const decimal_fast32_t dec2 {val2};

        const decimal_fast32_t res {dec1 / dec2};
        const decimal_fast32_t res_int {static_cast<double>(val1) / static_cast<double>(val2)};

        if (isinf(res) && isinf(res_int))
        {
        }
        else if (!BOOST_TEST(abs(res - res_int) < decimal_fast32_t(1, -1)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << static_cast<double>(val1) / static_cast<double>(val2) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge cases
    const decimal_fast32_t val1 {dist(rng)};
    const decimal_fast32_t zero {0, 0};
    BOOST_TEST(isnan(std::numeric_limits<decimal_fast32_t>::quiet_NaN() / dist(rng)));
    BOOST_TEST(isinf(std::numeric_limits<decimal_fast32_t>::infinity() / dist(rng)));
    BOOST_TEST(isnan(dist(rng) / std::numeric_limits<decimal_fast32_t>::quiet_NaN()));
    BOOST_TEST_EQ(abs(dist(rng) / std::numeric_limits<decimal_fast32_t>::infinity()), zero);
    BOOST_TEST(isinf(decimal_fast32_t(dist(rng)) / 0));
    BOOST_TEST(isinf(val1 / zero));
}

template <typename T>
void random_spot_division(T val1, T val2)
{
    const decimal_fast32_t dec1 {val1};
    const decimal_fast32_t dec2 {val2};

    const decimal_fast32_t res {dec1 / dec2};
    const decimal_fast32_t res_int {static_cast<float>(val1) / static_cast<float>(val2)};

    if (isinf(res) && isinf(res_int))
    {
    }
    else if (!BOOST_TEST(abs(res - res_int) < decimal_fast32_t(1, -2)))
    {
        // LCOV_EXCL_START
        std::cerr << "Val 1: " << val1
                  << "\nDec 1: " << dec1
                  << "\nVal 2: " << val2
                  << "\nDec 2: " << dec2
                  << "\nDec res: " << res
                  << "\nInt res: " << static_cast<float>(val1) / static_cast<float>(val2) << std::endl;
        // LCOV_EXCL_STOP
    }
}

int main()
{
    // Values that won't exceed the range of the significand
    // Only positive values
    random_addition(0, 5'000'000);
    random_addition(0L, 5'000'000L);
    random_addition(0LL, 5'000'000LL);
    random_mixed_addition(0, 5'000'000);
    random_mixed_addition(0L, 5'000'000L);
    random_mixed_addition(0LL, 5'000'000LL);

    // Only two negative values
    random_addition(-5'000'000, 0);
    random_addition(-5'000'000L, 0L);
    random_addition(-5'000'000LL, 0LL);
    random_mixed_addition(-5'000'000, 0);
    random_mixed_addition(-5'000'000L, 0L);
    random_mixed_addition(-5'000'000LL, 0LL);

    // Only positive values
    random_subtraction(0, 5'000'000);
    random_subtraction(0L, 5'000'000L);
    random_subtraction(0LL, 5'000'000LL);
    random_mixed_subtraction(0, 5'000'000);
    random_mixed_subtraction(0L, 5'000'000L);
    random_mixed_subtraction(0LL, 5'000'000LL);

    // Only two negative values
    random_subtraction(-5'000'000, 0);
    random_subtraction(-5'000'000L, 0L);
    random_subtraction(-5'000'000LL, 0LL);
    random_mixed_subtraction(-5'000'000, 0);
    random_mixed_subtraction(-5'000'000L, 0L);
    random_mixed_subtraction(-5'000'000LL, 0LL);

    // Mixed Values
    random_subtraction(-5'000'000, 5'000'000);
    random_subtraction(-5'000'000L, 5'000'000L);
    random_subtraction(-5'000'000LL, 5'000'000LL);
    random_mixed_subtraction(-5'000'000, 5'000'000);
    random_mixed_subtraction(-5'000'000L, 5'000'000L);
    random_mixed_subtraction(-5'000'000LL, 5'000'000LL);

    // Anything in range
    random_addition(-5'000'000, 5'000'000);
    random_addition(-5'000'000L, 5'000'000L);
    random_addition(-5'000'000LL, 5'000'000LL);
    random_mixed_addition(-5'000'000, 5'000'000);
    random_mixed_addition(-5'000'000L, 5'000'000L);
    random_mixed_addition(-5'000'000LL, 5'000'000LL);

    // Anything in the domain
    random_converted_addition(0, (std::numeric_limits<int>::max)() / 2);
    random_converted_addition((std::numeric_limits<int>::min)() / 2, 0);
    random_converted_addition((std::numeric_limits<int>::min)() / 2, (std::numeric_limits<int>::max)() / 2);

    // Positive values
    const auto sqrt_int_max = static_cast<int>(std::sqrt(static_cast<double>((std::numeric_limits<int>::max)())));

    random_spot_multiplication(4477, 4139);
    random_spot_multiplication(28270, 45750);
    random_spot_multiplication(2137, 3272);
    random_spot_multiplication(-26554, 22692);

    random_multiplication(0, 5'000);
    random_multiplication(0L, 5'000L);
    random_multiplication(0LL, 5'000LL);
    random_multiplication(0, sqrt_int_max);
    random_mixed_multiplication(0, 5'000);
    random_mixed_multiplication(0L, 5'000L);
    random_mixed_multiplication(0LL, 5'000LL);
    random_mixed_multiplication(0, sqrt_int_max);

    // Only negative values
    random_multiplication(-5'000, 0);
    random_multiplication(-5'000L, 0L);
    random_multiplication(-5'000LL, 0LL);
    random_multiplication(-sqrt_int_max, 0);
    random_mixed_multiplication(-5'000, 0);
    random_mixed_multiplication(-5'000L, 0L);
    random_mixed_multiplication(-5'000LL, 0LL);
    random_mixed_multiplication(-sqrt_int_max, 0);

    // Mixed values
    random_multiplication(-5'000, 5'000);
    random_multiplication(-5'000L, 5'000L);
    random_multiplication(-5'000LL, 5'000LL);
    random_multiplication(-sqrt_int_max, sqrt_int_max);
    random_mixed_multiplication(-5'000, 5'000);
    random_mixed_multiplication(-5'000L, 5'000L);
    random_mixed_multiplication(-5'000LL, 5'000LL);
    random_mixed_multiplication(-sqrt_int_max, sqrt_int_max);

    random_spot_division(-23984, 2561);

    random_division(0, 5'000);
    random_division(0L, 5'000L);
    random_division(0LL, 5'000LL);
    random_division(0, sqrt_int_max);
    random_mixed_division(0, 5'000);
    random_mixed_division(0L, 5'000L);
    random_mixed_division(0LL, 5'000LL);
    random_mixed_division(0, sqrt_int_max);

    // Only negative values
    random_division(-5'000, 0);
    random_division(-5'000L, 0L);
    random_division(-5'000LL, 0LL);
    random_division(-sqrt_int_max, 0);
    random_mixed_division(-5'000, 0);
    random_mixed_division(-5'000L, 0L);
    random_mixed_division(-5'000LL, 0LL);
    random_mixed_division(-sqrt_int_max, 0);

    // Mixed values
    random_division(-5'000, 5'000);
    random_division(-5'000L, 5'000L);
    random_division(-5'000LL, 5'000LL);
    random_division(-sqrt_int_max, sqrt_int_max);
    random_mixed_division(-5'000, 5'000);
    random_mixed_division(-5'000L, 5'000L);
    random_mixed_division(-5'000LL, 5'000LL);
    random_mixed_division(-sqrt_int_max, sqrt_int_max);

    return boost::report_errors();
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif


