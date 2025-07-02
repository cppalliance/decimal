// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal64_fast.hpp>
#include <boost/decimal/iostream.hpp>
#include <boost/decimal/cmath.hpp>
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

template <typename T>
void random_LT(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 < dec2, val1 < val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge cases
    BOOST_TEST(decimal64_fast(dist(rng)) < std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(!(decimal64_fast(dist(rng)) < -std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!(decimal64_fast(dist(rng)) < std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!(std::numeric_limits<decimal64_fast>::quiet_NaN() < std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_mixed_LT(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const T dec2 {static_cast<T>(decimal64_fast(val2))};

        if (!BOOST_TEST_EQ(dec1 < dec2, val1 < val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Reverse order of the operands
    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const T dec1 {static_cast<T>(decimal64_fast(val1))};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 < dec2, val1 < val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge Cases
    BOOST_TEST_EQ(decimal64_fast(1) < T(1), false);
    BOOST_TEST_EQ(decimal64_fast(10) < T(10), false);
    BOOST_TEST_EQ(T(1) < decimal64_fast(1), false);
    BOOST_TEST_EQ(T(10) < decimal64_fast(10), false);
    BOOST_TEST_EQ(BOOST_DECIMAL_DEC_INFINITY < T(1), false);
    BOOST_TEST_EQ(-BOOST_DECIMAL_DEC_INFINITY < T(1), true);
    BOOST_TEST_EQ(BOOST_DECIMAL_DEC_NAN < T(1), false);
}

template <typename T>
void random_LE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 <= dec2, val1 <= val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(decimal64_fast(dist(rng)) <= std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(!(decimal64_fast(dist(rng)) <= -std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!(decimal64_fast(dist(rng)) <= std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!(std::numeric_limits<decimal64_fast>::quiet_NaN() <= std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_mixed_LE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const T dec2 {static_cast<T>(decimal64_fast(val2))};

        if (!BOOST_TEST_EQ(dec1 <= dec2, val1 <= val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(dist(rng) <= std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(!(dist(rng) <= -std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!(dist(rng) <= std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_GT(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 > dec2, val1 > val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    const auto inf = std::numeric_limits<decimal64_fast>::infinity();
    BOOST_TEST(!(decimal64_fast(dist(rng)) > inf));
    BOOST_TEST((decimal64_fast(dist(rng)) > -inf));
    BOOST_TEST(!(decimal64_fast(dist(rng)) > std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!(std::numeric_limits<decimal64_fast>::quiet_NaN() > std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_mixed_GT(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const T dec2 {static_cast<T>(decimal64_fast(val2))};

        if (!BOOST_TEST_EQ(dec1 > dec2, val1 > val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(!(dist(rng) > std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST((dist(rng) > -std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!(dist(rng) > std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_GE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 >= dec2, val1 >= val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(!(decimal64_fast(dist(rng)) >= std::numeric_limits<decimal64_fast>::infinity()));

    // MSVC 14.2
    #if !defined(_MSC_VER)
    BOOST_TEST((decimal64_fast(dist(rng)) >= -std::numeric_limits<decimal64_fast>::infinity()));
    #endif

    BOOST_TEST(!(decimal64_fast(dist(rng)) >= std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!(std::numeric_limits<decimal64_fast>::quiet_NaN() >= std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_mixed_GE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const T dec2 {static_cast<T>(decimal64_fast(val2))};

        if (!BOOST_TEST_EQ(dec1 >= dec2, val1 >= val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(!(dist(rng) >= std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST((dist(rng) >= -std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!(dist(rng) >= std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_EQ(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 == dec2, val1 == val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(!(std::numeric_limits<decimal64_fast>::quiet_NaN() == std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_mixed_EQ(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const T dec2 {static_cast<T>(decimal64_fast(val2))};

        if (!BOOST_TEST_EQ(dec1 == dec2, val1 == val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const T dec1 {static_cast<T>(decimal64_fast(val1))};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 == dec2, val1 == val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge Cases
    BOOST_TEST_EQ(decimal64_fast(1), T(1));
    BOOST_TEST_EQ(decimal64_fast(10), T(10));
    BOOST_TEST_EQ(decimal64_fast(100), T(100));
    BOOST_TEST_EQ(decimal64_fast(1000), T(1000));
    BOOST_TEST_EQ(decimal64_fast(10000), T(10000));
    BOOST_TEST_EQ(decimal64_fast(100000), T(100000));
    BOOST_TEST_EQ(BOOST_DECIMAL_DEC_NAN == T(1), false);
    BOOST_TEST_EQ(BOOST_DECIMAL_DEC_INFINITY == T(1), false);
}

template <typename T>
void random_NE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST_EQ(dec1 != dec2, val1 != val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST((std::numeric_limits<decimal64_fast>::quiet_NaN() != std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

template <typename T>
void random_mixed_NE(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const T dec2 {static_cast<T>(decimal64_fast(val2))};

        if (!BOOST_TEST_EQ(dec1 != dec2, val1 != val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
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

        const decimal64_fast dec1 {val1};
        const decimal64_fast dec2 {val2};

        if (!BOOST_TEST((dec1 <=> dec2) == (val1 <=> val2)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST((decimal64_fast(dist(rng)) <=> std::numeric_limits<decimal64_fast>::quiet_NaN()) == std::partial_ordering::unordered);
    BOOST_TEST((std::numeric_limits<decimal64_fast>::quiet_NaN() <=> std::numeric_limits<decimal64_fast>::quiet_NaN()) == std::partial_ordering::unordered);
}

template <typename T>
void random_mixed_SPACESHIP(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal64_fast dec1 {val1};
        const T dec2 {static_cast<T>(decimal64_fast(val2))};

        if (!BOOST_TEST((dec1 <=> dec2) == (val1 <=> val2)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    if (!BOOST_TEST((dist(rng) <=> std::numeric_limits<decimal64_fast>::quiet_NaN()) == std::partial_ordering::unordered))
    {
        // LCOV_EXCL_START
        const auto eval {dist(rng) <=> std::numeric_limits<decimal64_fast>::quiet_NaN()};
        if (eval == std::partial_ordering::less)
            std::cerr << "Less" << std::endl;
        else if (eval == std::partial_ordering::greater)
            std::cerr << "Greater" << std::endl;
        else if (eval == std::partial_ordering::equivalent)
            std::cerr << "Equivalent" << std::endl;
        else
            std::cerr << "Unordered" << std::endl;
        // LCOV_EXCL_STOP
    }

    BOOST_TEST((std::numeric_limits<decimal64_fast>::quiet_NaN() <=> std::numeric_limits<decimal64_fast>::quiet_NaN()) == std::partial_ordering::unordered);
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

    random_mixed_LT(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_mixed_LT(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_mixed_LT(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_mixed_LT(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_mixed_LT(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_mixed_LT(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_LE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_LE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_LE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_LE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_LE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_LE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_mixed_LE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_mixed_LE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_mixed_LE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_mixed_LE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_mixed_LE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_mixed_LE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_GT(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_GT(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_GT(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_GT(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_GT(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_GT(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_mixed_GT(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_mixed_GT(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_mixed_GT(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_mixed_GT(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_mixed_GT(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_mixed_GT(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_GE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_GE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_GE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_GE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_GE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_GE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_mixed_GE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_mixed_GE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_mixed_GE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_mixed_GE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_mixed_GE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_mixed_GE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_EQ(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_EQ(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_EQ(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_EQ(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_EQ(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_EQ(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_mixed_EQ(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_mixed_EQ(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_mixed_EQ(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_mixed_EQ(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_mixed_EQ(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_mixed_EQ(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_NE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_NE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_NE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_NE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_NE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_NE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_mixed_NE(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_mixed_NE(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_mixed_NE(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_mixed_NE(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_mixed_NE(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_mixed_NE(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    random_SPACESHIP(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_SPACESHIP(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_SPACESHIP(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_SPACESHIP(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_SPACESHIP(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_SPACESHIP(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());

    random_mixed_SPACESHIP(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    random_mixed_SPACESHIP(std::numeric_limits<unsigned>::min(), std::numeric_limits<unsigned>::max());
    random_mixed_SPACESHIP(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    random_mixed_SPACESHIP(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());
    random_mixed_SPACESHIP(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
    random_mixed_SPACESHIP(std::numeric_limits<unsigned long long>::min(), std::numeric_limits<unsigned long long>::max());
    #endif

    constexpr auto pos_zero = boost::decimal::decimal64_fast{0, 0, false};
    constexpr auto neg_zero = boost::decimal::decimal64_fast{0, 0, true};
    BOOST_TEST_EQ(pos_zero, neg_zero);

    return boost::report_errors();
}
