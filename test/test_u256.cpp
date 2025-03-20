// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/core/lightweight_test.hpp>
#include <boost/decimal/detail/u256.hpp>
#include <boost/decimal/detail/emulated256.hpp>
#include <random>

static std::mt19937_64 rng(42);

constexpr std::size_t N = 1024;

using namespace boost::decimal::detail;

void test_equality()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFFFFFFFFFFFFFFULL);

    // one word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;

        BOOST_TEST_EQ(new_val_1 == new_val_1, old_val_1 == old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;

        BOOST_TEST_EQ(new_val_1 == new_val_2, old_val_1 == old_val_2);
    }

    // two word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;

        BOOST_TEST_EQ(new_val_1 == new_val_1, old_val_1 == old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;

        BOOST_TEST_EQ(new_val_1 == new_val_2, old_val_1 == old_val_2);
    }

    // three word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;

        BOOST_TEST_EQ(new_val_1 == new_val_1, old_val_1 == old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;

        BOOST_TEST_EQ(new_val_1 == new_val_2, old_val_1 == old_val_2);
    }

    // four word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        new_val_1.bytes[3] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;
        old_val_1.high.high = num;

        BOOST_TEST_EQ(new_val_1 == new_val_1, old_val_1 == old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        new_val_2.bytes[3] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;
        old_val_2.high.high = num;

        BOOST_TEST_EQ(new_val_1 == new_val_2, old_val_1 == old_val_2);
    }
}

void test_inequality()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFFFFFFFFFFFFFFULL);

    // one word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;

        BOOST_TEST_EQ(new_val_1 != new_val_1, old_val_1 != old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;

        BOOST_TEST_EQ(new_val_1 != new_val_2, old_val_1 != old_val_2);
    }

    // two word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;

        BOOST_TEST_EQ(new_val_1 != new_val_1, old_val_1 != old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;

        BOOST_TEST_EQ(new_val_1 != new_val_2, old_val_1 != old_val_2);
    }

    // three word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;

        BOOST_TEST_EQ(new_val_1 != new_val_1, old_val_1 != old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;

        BOOST_TEST_EQ(new_val_1 != new_val_2, old_val_1 != old_val_2);
    }

    // four word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        new_val_1.bytes[3] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;
        old_val_1.high.high = num;

        BOOST_TEST_EQ(new_val_1 != new_val_1, old_val_1 != old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        new_val_2.bytes[3] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;
        old_val_2.high.high = num;

        BOOST_TEST_EQ(new_val_1 != new_val_2, old_val_1 != old_val_2);
    }
}

void test_less()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFFFFFFFFFFFFFFULL);

    // one word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;

        BOOST_TEST_EQ(new_val_1 < new_val_1, old_val_1 < old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;

        BOOST_TEST_EQ(new_val_1 < new_val_2, old_val_1 < old_val_2);
    }

    // two word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;

        BOOST_TEST_EQ(new_val_1 < new_val_1, old_val_1 < old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;

        BOOST_TEST_EQ(new_val_1 < new_val_2, old_val_1 < old_val_2);
    }

    // three word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;

        BOOST_TEST_EQ(new_val_1 < new_val_1, old_val_1 < old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;

        BOOST_TEST_EQ(new_val_1 < new_val_2, old_val_1 < old_val_2);
    }

    // four word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        new_val_1.bytes[3] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;
        old_val_1.high.high = num;

        BOOST_TEST_EQ(new_val_1 < new_val_1, old_val_1 < old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        new_val_2.bytes[3] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;
        old_val_2.high.high = num;

        BOOST_TEST_EQ(new_val_1 < new_val_2, old_val_1 < old_val_2);
    }
}

void test_less_equal()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFFFFFFFFFFFFFFULL);

    // one word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_1, old_val_1 <= old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_2, old_val_1 <= old_val_2);
    }

    // two word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_1, old_val_1 <= old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_2, old_val_1 <= old_val_2);
    }

    // three word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_1, old_val_1 <= old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_2, old_val_1 <= old_val_2);
    }

    // four word
    for (std::size_t i {}; i < N; ++i)
    {
        std::uint64_t num = dist(rng);
        u256 new_val_1;
        new_val_1.bytes[0] = num;
        new_val_1.bytes[1] = num;
        new_val_1.bytes[2] = num;
        new_val_1.bytes[3] = num;
        uint256_t old_val_1;
        old_val_1.low.low = num;
        old_val_1.low.high = num;
        old_val_1.high.low = num;
        old_val_1.high.high = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_1, old_val_1 <= old_val_1);

        num = dist(rng);
        u256 new_val_2;
        new_val_2.bytes[0] = num;
        new_val_2.bytes[1] = num;
        new_val_2.bytes[2] = num;
        new_val_2.bytes[3] = num;
        uint256_t old_val_2;
        old_val_2.low.low = num;
        old_val_2.low.high = num;
        old_val_2.high.low = num;
        old_val_2.high.high = num;

        BOOST_TEST_EQ(new_val_1 <= new_val_2, old_val_1 <= old_val_2);
    }
}

int main()
{
    test_equality();
    test_inequality();
    test_less();
    test_less_equal();

    return boost::report_errors();
}
