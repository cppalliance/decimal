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

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4146)
#endif

template <typename T>
void random_addition(T lower, T upper)
{
    std::mt19937_64 rng(42);
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
    std::mt19937_64 rng(42);
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
    std::mt19937_64 rng(42);
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
    std::mt19937_64 rng(42);
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

    BOOST_TEST(isinf(std::numeric_limits<decimal64>::infinity() - dist(rng)));
    BOOST_TEST(isinf(dist(rng) - std::numeric_limits<decimal64>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64>::quiet_NaN() - dist(rng)));
    BOOST_TEST(isnan(dist(rng) - std::numeric_limits<decimal64>::quiet_NaN()));
}

int main()
{
    // Values that won't exceed the range of the significand
    // Only positive values
    random_addition(0, 5'000'000);
    random_addition(0LL, 4'000'000'000'000'000LL);
    random_mixed_addition(0, 5'000'000);
    random_mixed_addition(0LL, 4'000'000'000'000'000LL);

    // Only two negative values
    random_addition(-5'000'000, 0);
    //random_addition(-4'000'000'000'000'000LL, 0LL);
    random_mixed_addition(-5'000'000, 0);
    //random_mixed_addition(4'000'000'000'000'000LL, 0LL);

    // Subtraction
    random_subtraction(0, 5'000'000);
    //random_subtraction(0LL, 4'000'000'000'000'000LL);
    random_mixed_subtraction(0, 5'000'000);
    //random_mixed_subtraction(0LL, 4'000'000'000'000'000LL);

    // Only two negative values
    random_subtraction(-5'000'000, 0);
    //random_subtraction(-4'000'000'000'000'000LL, 0LL);
    random_mixed_subtraction(-5'000'000, 0);
    //random_mixed_subtraction(4'000'000'000'000'000LL, 0LL);

    // Mixed values
    random_subtraction(-5'000'000, 5'000'000);
    //random_subtraction(-4'000'000'000'000'000LL, 4'000'000'000'000'000LL);
    random_mixed_subtraction(-5'000'000, 5'000'000);
    //random_mixed_subtraction(-4'000'000'000'000'000LL, 4'000'000'000'000'000LL);

    return boost::report_errors();
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
