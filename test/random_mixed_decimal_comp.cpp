// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>

using namespace boost::decimal;

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(1024U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(1024U >> 4U); // Number of trials
#endif

// NOLINTNEXTLINE : Seed with a constant for repeatability
static std::mt19937_64 rng(42); // NOSONAR : Global rng is not const

void random_mixed_EQ()
{
    std::uniform_int_distribution<int> dist(-9'999'999, 9'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const int val1 {dist(rng)};
        const int val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal64 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 == dec2, val1 == val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const int val1 {dist(rng)};
        const int val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 == dec2, val1 == val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }

    // Edge cases
    const auto guarantee {dist(rng)};
    if (!BOOST_TEST(decimal64{guarantee} == decimal32{guarantee}))
    {
        std::cerr << "Dec64: " << decimal64{guarantee}
                  << "\nDec32: " << decimal32{guarantee} << std::endl;
    }

    BOOST_TEST_EQ(std::numeric_limits<decimal32>::quiet_NaN() == decimal64(dist(rng)), false);
    BOOST_TEST_EQ(std::numeric_limits<decimal64>::quiet_NaN() == decimal32(dist(rng)), false);
    BOOST_TEST_EQ(std::numeric_limits<decimal32>::infinity() == decimal64(dist(rng)), false);
    BOOST_TEST_EQ(std::numeric_limits<decimal64>::infinity() == decimal32(dist(rng)), false);
}

void random_mixed_NE()
{
    std::uniform_int_distribution<int> dist(-9'999'999, 9'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const int val1 {dist(rng)};
        const int val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal64 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 != dec2, val1 != val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const int val1 {dist(rng)};
        const int val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 != dec2, val1 != val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }

    // Edge cases
    BOOST_TEST(std::numeric_limits<decimal32>::quiet_NaN() != decimal64(dist(rng)));
    BOOST_TEST(std::numeric_limits<decimal64>::quiet_NaN() != decimal32(dist(rng)));
    BOOST_TEST(std::numeric_limits<decimal32>::infinity() != decimal64(dist(rng)));
    BOOST_TEST(std::numeric_limits<decimal64>::infinity() != decimal32(dist(rng)));
}

void random_mixed_LT()
{
    std::uniform_int_distribution<int> dist(-9'999'999, 9'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const int val1 {dist(rng)};
        const int val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal64 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 < dec2, val1 < val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const int val1 {dist(rng)};
        const int val2 {dist(rng)};

        const decimal64 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 < dec2, val1 < val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }

    // Edge cases
    BOOST_TEST_EQ(std::numeric_limits<decimal32>::quiet_NaN() < decimal64(dist(rng)), false);
    BOOST_TEST_EQ(std::numeric_limits<decimal64>::quiet_NaN() < decimal32(dist(rng)), false);
    BOOST_TEST_EQ(std::numeric_limits<decimal32>::infinity() < decimal64(dist(rng)), false);
    BOOST_TEST_EQ(std::numeric_limits<decimal64>::infinity() < decimal32(dist(rng)), false);
}

int main()
{
    random_mixed_EQ();
    random_mixed_NE();
    random_mixed_LT();

    return boost::report_errors();
}
