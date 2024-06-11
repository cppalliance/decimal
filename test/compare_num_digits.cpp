// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <random>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <cassert>
#include <boost/decimal/detail/countl.hpp>

template <typename Func>
void test(Func f, const char* title)
{
    using namespace std::chrono_literals;

    std::mt19937_64 rng(42);

    // Check in ranges
    std::uniform_int_distribution<std::uint32_t> two(10, 99);
    std::uniform_int_distribution<std::uint32_t> three(100, 999);
    std::uniform_int_distribution<std::uint32_t> four(1000, 9999);
    std::uniform_int_distribution<std::uint32_t> five(10000, 99999);
    std::uniform_int_distribution<std::uint32_t> six(100000, 999999);
    std::uniform_int_distribution<std::uint32_t> seven(1000000, 9999999);
    std::uniform_int_distribution<std::uint32_t> eight(10000000, 99999999);
    std::uniform_int_distribution<std::uint32_t> nine(100000000, 999999999);

    std::size_t discard {};

    const auto t1 = std::chrono::steady_clock::now();

    for (std::size_t loops {}; loops < 100; ++loops)
    {
        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(two(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 2);
        }

        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(three(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 3);
        }

        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(four(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 4);
        }

        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(five(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 5);
        }

        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(six(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 6);
        }

        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(seven(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 7);
        }

        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(eight(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 8);
        }

        for (std::size_t i {}; i < 100; ++i)
        {
            const auto res {f(nine(rng))};
            discard += static_cast<std::size_t>(res);
            assert(res == 9);
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << title << ": " << ( t2 - t1 ) / 1ns << " ns (s=" << discard << ")\n";
}

constexpr auto naive(std::uint32_t x) noexcept -> int
{
    int digits = 0;

    while (x)
    {
        x /= UINT32_C(10);
        ++digits;
    }

    return digits;
}

constexpr auto binary_search(std::uint32_t x) noexcept -> int
{
    if (x >= UINT32_C(10000))
    {
        if (x >= UINT32_C(10000000))
        {
            if (x >= UINT32_C(100000000))
            {
                if (x >= UINT32_C(1000000000))
                {
                    return 10;
                }
                return 9;
            }
            return 8;
        }

        else if (x >= UINT32_C(100000))
        {
            if (x >= UINT32_C(1000000))
            {
                return 7;
            }
            return 6;
        }
        return 5;
    }
    else if (x >= UINT32_C(100))
    {
        if (x >= UINT32_C(1000))
        {
            return 4;
        }
        return 3;
    }
    else if (x >= UINT32_C(10))
    {
        return 2;
    }

    return 1;
}

constexpr auto linear_search(std::uint32_t x) noexcept -> int
{
    auto r = (x >= 1000000000) ? 10 : (x >= 100000000) ? 9 : (x >= 10000000) ? 8 :
                            (x >= 1000000) ? 7 : (x >= 100000) ? 6 : (x >= 10000) ? 5 :
                                     (x >= 1000) ? 4 : (x >= 100) ? 3 : (x >= 10) ? 2 : 1;
    return r;
}

constexpr auto base_two_dig(std::uint32_t x) noexcept -> int
{
    return x ? 32 - boost::decimal::detail::countl_zero(x) : 0;
}

static constexpr std::uint8_t guess[33] = {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
        6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
        9, 9, 9
};

static constexpr std::uint32_t tens[] = {
        1, 10, 100, 1000, 10000, 100000,
        1000000, 10000000, 100000000, 1000000000,
};

constexpr auto log2_digits(std::uint32_t x) -> int
{
    const auto digits {guess[base_two_dig(x)]};
    return digits + (x >= tens[digits]);
}

int main()
{
    test(naive, "        Naive");
    test(binary_search, "Binary Search");
    test(linear_search, "Linear Search");
    test(log2_digits, "          Log");

    std::cout << std::endl;

    return 1;
}

/*
Output as run on Apple M1 with -march=native -O3 -std=c++23
====== BEGIN OUTPUT ======
        Naive: 2400667 ns (s=440000)
Binary Search: 2287125 ns (s=440000)
Linear Search: 2047792 ns (s=440000)
          Log: 1982542 ns (s=440000)

EXIT STATUS: 1
====== END OUTPUT ======
 */
