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
void random_LT(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 < dec2, val1 < val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }
}

template <typename T>
void random_LE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 <= dec2, val1 <= val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }
}

template <typename T>
void random_GT(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 > dec2, val1 > val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }
}

template <typename T>
void random_GE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 >= dec2, val1 >= val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }
}

template <typename T>
void random_EQ(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 == dec2, val1 == val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }
}

template <typename T>
void random_NE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 != dec2, val1 != val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
template <typename T>
void random_SPACESHIP(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        if (!BOOST_TEST((dec1 <=> dec2) == (val1 <=> val2)))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
        }
    }
}
#endif

int main()
{
    random_LT(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_LT(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_LT(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_LT(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_LT(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_LT(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_LE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_LE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_LE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_LE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_LE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_LE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_GT(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_GT(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_GT(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_GT(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_GT(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_GT(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_GE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_GE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_GE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_GE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_GE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_GE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_EQ(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_EQ(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_EQ(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_EQ(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_EQ(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_EQ(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_NE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_NE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_NE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_NE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_NE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_NE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    random_SPACESHIP(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_SPACESHIP(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_SPACESHIP(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_SPACESHIP(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_SPACESHIP(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_SPACESHIP(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());
    #endif

    return boost::report_errors();
}
