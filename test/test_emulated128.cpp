// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>
#include <climits>

using namespace boost::decimal;

#ifdef BOOST_DECIMAL_HAS_INT128

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(1024U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(1024U >> 4U); // Number of trials
#endif

static std::mt19937_64 rng(42);

void random_mixed_addition()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        const detail::uint128 res = emulated1 + emulated2;
        const auto res_int = static_cast<detail::uint128_t>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
        }
    }
}

void random_mixed_subtraction()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        const detail::uint128 res = emulated1 - emulated2;
        const auto res_int = static_cast<detail::uint128_t>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
        }
    }
}

void random_mixed_multiplication()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        const detail::uint128 res = emulated1 * emulated2;
        const auto res_int = static_cast<detail::uint128_t>(res);

        if (!BOOST_TEST_EQ(res_int, val1 * val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
        }
    }
}

void random_mixed_division()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        const detail::uint128 res = emulated1 / emulated2;
        const auto res_int = static_cast<detail::uint128_t>(res);

        if (!BOOST_TEST_EQ(res_int, val1 / val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 / val2 << std::endl;
        }
    }
}

void random_mixed_LT()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 < emulated2, val1 < val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_LE()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 <= emulated2, val1 <= val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_GT()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 > emulated2, val1 > val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_GE()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 >= emulated2, val1 >= val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_EQ()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 == emulated2, val1 == val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_NE()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 != emulated2, val1 != val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_and()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 & emulated2, val1 & val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_or()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 | emulated2, val1 | val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

void random_mixed_xor()
{
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    for (std::size_t i {}; i < N; ++i)
    {
        const detail::uint128_t val1 {dist(rng)};
        const detail::uint128_t val2 {dist(rng)};

        const detail::uint128 emulated1 {val1};
        const detail::uint128 emulated2 {val2};

        if (!BOOST_TEST_EQ(emulated1 ^ emulated2, val1 ^ val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
        }
    }
}

int main()
{
    random_mixed_addition();
    random_mixed_subtraction();
    random_mixed_multiplication();
    random_mixed_division();

    random_mixed_LT();
    random_mixed_LE();
    random_mixed_GT();
    random_mixed_GE();
    random_mixed_EQ();
    random_mixed_NE();

    random_mixed_and();
    random_mixed_or();
    random_mixed_xor();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif //BOOST_DECIMAL_HAS_INT128
