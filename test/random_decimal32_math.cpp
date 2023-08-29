// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>

using namespace boost::decimal;

static constexpr auto N {1024U};

// NOLINTNEXTLINE : Seed with a constant for repeatability
static std::mt19937_64 rng(42); // NOSONAR : Global rng is not const

template <typename T>
void random_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        const decimal32 res = dec1 + dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
        }
    }
}

template <typename T>
void random_mixed_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal32(val2))};

        const decimal32 res = dec1 + trunc_val_2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << trunc_val_2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
        }
    }
}

template <typename T>
void random_converted_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        // Convert these to and from to ensure rounding
        const T val1 {static_cast<T>(decimal32(dist(rng)))};
        const T val2 {static_cast<T>(decimal32(dist(rng)))};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        const decimal32 res {dec1 + dec2};
        const decimal32 comp_val {val1 + val2};

        if (!BOOST_TEST_EQ(res, comp_val))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << comp_val << std::endl;
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

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        const decimal32 res = dec1 - dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
        }
    }
}

template <typename T>
void random_mixed_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal32(val2))};

        const decimal32 res = dec1 - trunc_val_2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << trunc_val_2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
        }
    }
}

template <typename T>
void random_multiplication(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        const decimal32 res {dec1 * dec2};
        const decimal32 res_int {val1 * val2};

        if (!BOOST_TEST_EQ(res, res_int))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
        }
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

        const decimal32 dec1 {val1};
        const T dec2 {static_cast<T>(decimal32(val2))};

        const decimal32 res {dec1 * dec2};
        const decimal32 res_int {val1 * val2};

        if (!BOOST_TEST_EQ(res, res_int))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
        }
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

    return boost::report_errors();
}
