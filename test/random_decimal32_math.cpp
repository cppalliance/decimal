// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>
#include <cmath>
#include <cerrno>

using namespace boost::decimal;

static constexpr auto N {10U};

template <typename T>
void random_addition(T lower, T upper)
{
    std::mt19937_64 rng(42);
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
                      << "\nInt res: " << res_int << std::endl;
        }
    }
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
                      << "\nInt res: " << res_int << std::endl;
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

    // Only two negative values
    random_addition(-5'000'000, 0);
    random_addition(-5'000'000L, 0L);
    random_addition(-5'000'000LL, 0LL);

    // Only positive values
    random_subtraction(0, 5'000'000);
    random_subtraction(0L, 5'000'000L);
    random_subtraction(0LL, 5'000'000LL);

    // Only two negative values
    random_subtraction(-5'000'000, 0);
    random_subtraction(-5'000'000L, 0L);
    random_subtraction(-5'000'000LL, 0LL);

    return boost::report_errors();
}
