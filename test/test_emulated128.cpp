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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 / val2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << emulated1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << emulated2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void pow_10()
{
    detail::uint128 our_128 {10};
    detail::uint128 our_128_64_bit_mul {10};
    detail::uint128_t builtin_128 {10};

    BOOST_TEST_EQ(our_128, builtin_128);
    for (int i = 1; i < 38; ++i)
    {
        our_128 *= detail::uint128(10);
        our_128_64_bit_mul *= UINT64_C(10);
        builtin_128 *= 10;
        BOOST_TEST_EQ(our_128, builtin_128);
        BOOST_TEST_EQ(our_128, our_128_64_bit_mul);
    }

    for (int i = 1; i < 38; ++i)
    {
        our_128 /= detail::uint128(10);
        our_128_64_bit_mul /= UINT64_C(10);
        builtin_128 /= 10;
        BOOST_TEST_EQ(our_128, builtin_128);
        BOOST_TEST_EQ(our_128, our_128_64_bit_mul);
    }
}

void stream()
{
    detail::uint128 our_128 {10};
    detail::uint128_t builtin_128 {10};

    BOOST_TEST_EQ(our_128, builtin_128);
    for (int i = 1; i < 30; ++i)
    {
        our_128 *= detail::uint128(10);
        builtin_128 *= 10;

        std::stringstream ss1;
        ss1 << our_128;
        std::stringstream ss2;
        ss2 << builtin_128;

        BOOST_TEST_EQ(ss1.str(), ss2.str());
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

    pow_10();

    stream();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif //BOOST_DECIMAL_HAS_INT128
