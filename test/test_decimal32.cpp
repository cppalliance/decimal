// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <bitset>
#include <limits>

using namespace boost::decimal;

void test_comp()
{
    constexpr decimal32 small(1, -50);
    BOOST_TEST(small == small);

    constexpr decimal32 sig(123456, -50);
    BOOST_TEST(sig != small);
}

void test_constructor()
{
    decimal32 one(0b1, -100);
    // 0 for sign
    // 00000 for combination field
    // 000001 for exp
    // 0000000000'0000000001 for significand
    BOOST_TEST_EQ(std::bitset<32>(to_bits(one)), std::bitset<32>(0b0'00000'000001'0000000000'0000000001));

    decimal32 neg_one(-0b1, -100);
    // 1 for sign
    // 00000 for combination field
    // 000001 for exp
    // 0000000000'0000000001 for significand
    BOOST_TEST_EQ(std::bitset<32>(to_bits(neg_one)), std::bitset<32>(0b1'00000'000001'0000000000'0000000001));

    decimal32 big_sig(0b1111111111'1111111111, -100);
    // 0 for sign
    // 00000 for combination field
    // 000001 for exp
    // 1111111111'1111111111 for significand
    BOOST_TEST_EQ(std::bitset<32>(to_bits(big_sig)), std::bitset<32>(0b0'00000'000001'1111111111'1111111111));

    // Test the combination field
    decimal32 one_e_max(0b1, 90);
    // 0 for sign
    // 10000 for combination field
    // 111111 for exp
    // 0000000000'0000000001 for significand
    BOOST_TEST_EQ(std::bitset<32>(to_bits(one_e_max)), std::bitset<32>(0b0'10000'111111'0000000000'0000000001));
}

void test_non_finite_values()
{
    constexpr decimal32 one(0b1, -100);

    BOOST_TEST(std::numeric_limits<decimal32>::has_infinity);
    BOOST_TEST(isinf(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!isinf(one));

    BOOST_TEST(std::numeric_limits<decimal32>::has_quiet_NaN);
    BOOST_TEST(std::numeric_limits<decimal32>::has_signaling_NaN);
    BOOST_TEST(isnan(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(isnan(std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!isnan(one));

    BOOST_TEST(!issignaling(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(issignaling(std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!issignaling(one));

    BOOST_TEST(isfinite(one));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::signaling_NaN()));
}

void test_unary_arithmetic()
{
    constexpr decimal32 one(0b1, -100);
    BOOST_TEST(+one == one);
    BOOST_TEST(-one != one);
}

int main()
{
    test_comp();
    test_constructor();
    test_non_finite_values();
    test_unary_arithmetic();

    return boost::report_errors();
}
