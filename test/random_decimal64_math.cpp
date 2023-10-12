// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>
#include <climits>

using namespace boost::decimal;

static constexpr auto N {1024U};

static std::mt19937_64 rng(42);

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4146)
#endif

template <typename T>
void random_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const decimal64 dec2 {val2};

        const decimal64 res = dec1 + dec2;
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

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() + decimal64{0,0}));
    BOOST_TEST(isinf(decimal64{0,0} + std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() + decimal64{0,0}));
    BOOST_TEST(isnan(decimal64{0,0} + std::numeric_limits<decimal64>::quiet_NaN()));
}

template <typename T>
void random_mixed_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal64(val2))};

        const decimal64 res = dec1 + trunc_val_2;
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

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() + dist(rng)));
    BOOST_TEST(isinf(dist(rng) + std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() + dist(rng)));
    BOOST_TEST(isnan(dist(rng) + std::numeric_limits<decimal64>::quiet_NaN()));
}

template <typename T>
void random_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const decimal64 dec2 {val2};

        const decimal64 res = dec1 - dec2;
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

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() - decimal64{0,0}));
    BOOST_TEST(isinf(decimal64{0,0} - std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() - decimal64{0,0}));
    BOOST_TEST(isnan(decimal64{0,0} - std::numeric_limits<decimal64>::quiet_NaN()));
}

template <typename T>
void random_mixed_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal64(val2))};

        const decimal64 res = dec1 - trunc_val_2;
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

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const T trunc_val_1 {static_cast<T>(decimal64(val1))};
        const decimal64 dec2 {val2};

        const decimal64 res = trunc_val_1 - dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << trunc_val_1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() - dist(rng)));
    BOOST_TEST(isinf(dist(rng) - std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() - dist(rng)));
    BOOST_TEST(isnan(dist(rng) - std::numeric_limits<decimal64>::quiet_NaN()));
}

template <typename T>
void spot_check_sub(T lhs, T rhs)
{
    const decimal64 dec1 {lhs};
    const decimal64 dec2 {rhs};
    const decimal64 res {dec1 - dec2};
    const auto res_int {static_cast<T>(res)};

    if (!BOOST_TEST_EQ(res_int, lhs - rhs))
    {
        std::cerr << "Val 1: " << lhs
                  << "\nDec 1: " << dec1
                  << "\nVal 2: " << rhs
                  << "\nDec 2: " << dec2
                  << "\nDec res: " << res
                  << "\nInt res: " << lhs - rhs << std::endl;
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

        const decimal64 dec1 {val1};
        const decimal64 dec2 {val2};

        const decimal64 res {dec1 * dec2};
        const decimal64 res_int {val1 * val2};

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

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() * decimal64(dist(rng))));
    BOOST_TEST(isinf(decimal64(dist(rng)) * std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() * decimal64(dist(rng))));
    BOOST_TEST(isnan(decimal64(dist(rng)) * std::numeric_limits<decimal64>::quiet_NaN()));
}

template <typename T>
void random_mixed_multiplication(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const T dec2 {static_cast<T>(decimal64(val2))};

        const decimal64 res {dec1 * dec2};
        const decimal64 res_int {val1 * val2};

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

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() * dist(rng)));
    BOOST_TEST(isinf(dist(rng) * std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() * dist(rng)));
    BOOST_TEST(isnan(dist(rng) * std::numeric_limits<decimal64>::quiet_NaN()));
}

template <typename T>
void random_division(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const decimal64 dec2 {val2};

        const decimal64 res {dec1 * dec2};
        const decimal64 res_int {val1 * val2};

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

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() / decimal64(dist(rng))));
    BOOST_TEST(!isinf(decimal64(dist(rng)) / std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() / decimal64(dist(rng))));
    BOOST_TEST(isnan(decimal64(dist(rng)) / std::numeric_limits<decimal64>::quiet_NaN()));
    BOOST_TEST(isnan(decimal64(dist(rng)) / decimal64(0)));
}

template <typename T>
void random_mixed_division(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const T dec2 {static_cast<T>(decimal64(val2))};

        const decimal64 res {dec1 * dec2};
        const decimal64 res_int {val1 * val2};

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

    // Edge cases
    const decimal64 val1 {dist(rng)};
    const decimal64 zero {0, 0};
    BOOST_TEST(isnan(val1 / zero));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() / dist(rng)));
    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() / dist(rng)));
    BOOST_TEST_EQ(decimal64(dist(rng)) / 0, 0);
}

int main()
{
    // Values that won't exceed the range of the significand
    // Only positive values
    random_addition(0, 5'000'000);
    random_addition(0LL, 4'000'000'000'000LL);
    random_mixed_addition(0, 5'000'000);
    random_mixed_addition(0LL, 4'000'000'000'000LL);

    // Only two negative values
    random_addition(-5'000'000, 0);
    random_addition(-4'000'000'000'000LL, 0LL);
    random_mixed_addition(-5'000'000, 0);
    random_mixed_addition(-4'000'000'000'000LL, 0LL);

    // Subtraction
    random_subtraction(0, 5'000'000);
    random_subtraction(0LL, 4'000'000'000'000LL);
    random_mixed_subtraction(0, 5'000'000);
    random_mixed_subtraction(0LL, 4'000'000'000'000LL);

    // Only two negative values
    random_subtraction(-5'000'000, 0);
    random_subtraction(-4'000'000'000'000LL, 0LL);
    random_mixed_subtraction(-5'000'000, 0);
    random_mixed_subtraction(-4'000'000'000'000LL, 0LL);

    // Mixed values
    random_subtraction(-5'000'000, 5'000'000);
    random_subtraction(-4'000'000'000'000LL, 4'000'000'000'000LL);
    random_mixed_subtraction(-5'000'000, 5'000'000);
    random_mixed_subtraction(-4'000'000'000'000LL, 4'000'000'000'000LL);

    // Multiplication
    const auto sqrt_int_max = static_cast<int>(std::sqrt(static_cast<double>((std::numeric_limits<int>::max)())));

    // Positive
    random_multiplication(0, 5'000);
    random_multiplication(0LL, 5'000LL);
    random_multiplication(0, sqrt_int_max);
    random_mixed_multiplication(0, 5'000);
    random_mixed_multiplication(0LL, 5'000LL);
    random_mixed_multiplication(0, sqrt_int_max);

    // Negative
    random_multiplication(-5'000, 0);
    random_multiplication(-5'000LL, 0LL);
    random_multiplication(-sqrt_int_max, 0);
    random_mixed_multiplication(-5'000, 0);
    random_mixed_multiplication(-5'000LL, 0LL);
    random_mixed_multiplication(-sqrt_int_max, 0);

    // Mixed
    random_multiplication(-5'000, 5'000);
    random_multiplication(-5'000LL, 5'000LL);
    random_multiplication(-sqrt_int_max, sqrt_int_max);
    random_mixed_multiplication(-5'000, 5'000);
    random_mixed_multiplication(-5'000LL, 5'000LL);
    random_mixed_multiplication(-sqrt_int_max, sqrt_int_max);

    // Division

    // Positive
    random_division(0, 5'000);
    random_division(0LL, 5'000LL);
    random_division(0, sqrt_int_max);
    random_mixed_division(0, 5'000);
    random_mixed_division(0LL, 5'000LL);
    random_mixed_division(0, sqrt_int_max);

    // Negative
    random_division(-5'000, 0);
    random_division(-5'000LL, 0LL);
    random_division(-sqrt_int_max, 0);
    random_mixed_division(-5'000, 0);
    random_mixed_division(-5'000LL, 0LL);
    random_mixed_division(-sqrt_int_max, 0);

    // Mixed
    random_division(-5'000, 5'000);
    random_division(-5'000LL, 5'000LL);
    random_division(-sqrt_int_max, sqrt_int_max);
    random_mixed_division(-5'000, 5'000);
    random_mixed_division(-5'000LL, 5'000LL);
    random_mixed_division(-sqrt_int_max, sqrt_int_max);

    // Spot checked values
    spot_check_sub(945501, 80);
    spot_check_sub(562, 998980);
    spot_check_sub(-954783, 746);
    spot_check_sub(513479119LL, 972535711690LL);

    return boost::report_errors();
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
