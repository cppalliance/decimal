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

    BOOST_TEST(small < sig);
    BOOST_TEST(small <= sig);
    BOOST_TEST(small <= small);
    BOOST_TEST(sig > small);
    BOOST_TEST(sig >= small);

    // Test cohorts
    BOOST_TEST(small == decimal32(10, -51));
    BOOST_TEST(small == decimal32(100, -52));
    BOOST_TEST(small == decimal32(1000, -53));
    BOOST_TEST(small == decimal32(10000, -54));
    BOOST_TEST(small == decimal32(100000, -55));
    BOOST_TEST(small == decimal32(1000000, -56));

    // Test non-finite comp
    BOOST_TEST(small < std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(small > -std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(!(small == std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(small != std::numeric_limits<decimal32>::infinity());

    BOOST_TEST(!(small < std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!(small < std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(small != std::numeric_limits<decimal32>::quiet_NaN());
    BOOST_TEST(std::numeric_limits<decimal32>::quiet_NaN() != std::numeric_limits<decimal32>::quiet_NaN());
}

void test_binary_constructor()
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

void test_decimal_constructor()
{
    // The significand is more than 7 digits
    // Apply correct rounding when in the range of 7 digits
    decimal32 big(123456789, 0);
    decimal32 rounded_big(1234568, 2);

    BOOST_TEST_EQ(big, rounded_big);
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

void test_addition()
{
    // Case 1: The difference is more than the digits of accuracy
    constexpr decimal32 big_num(0b1, 20);
    constexpr decimal32 small_num(0b1, -20);
    BOOST_TEST_EQ(big_num + small_num, big_num);
    BOOST_TEST_EQ(small_num + big_num, big_num);

    // Case 2: Round the last digit of the significand
    constexpr decimal32 full_length_num {10000000, 0};
    constexpr decimal32 rounded_full_length_num(10000000, 0);
    constexpr decimal32 no_round(1, -1);
    constexpr decimal32 round(9, -1);
    BOOST_TEST_EQ(full_length_num + no_round, full_length_num);
    BOOST_TEST_EQ(full_length_num + round, rounded_full_length_num);

    // Case 3: Add away
    constexpr decimal32 one(1, 0);
    constexpr decimal32 two(2, 0);
    constexpr decimal32 three(3, 0);

    BOOST_TEST_EQ(one + one, two);
    BOOST_TEST_EQ(two + one, three);
    BOOST_TEST_EQ(one + one + one, three);

    // Different orders of magnitude
    constexpr decimal32 ten(10, 0);
    constexpr decimal32 eleven(11, 0);
    BOOST_TEST_EQ(ten + one, eleven);

    constexpr decimal32 max_sig(9'999'999, 0);
    constexpr decimal32 max_plus_one(10'000'000, 0);
    BOOST_TEST_EQ(max_sig + one, max_plus_one);

    // Non-finite values
    constexpr decimal32 qnan_val(std::numeric_limits<decimal32>::quiet_NaN());
    constexpr decimal32 snan_val(std::numeric_limits<decimal32>::signaling_NaN());
    constexpr decimal32 inf_val(std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(isnan(qnan_val + one));
    BOOST_TEST(isnan(snan_val + one));
    BOOST_TEST(isnan(one + qnan_val));
    BOOST_TEST(isnan(one + snan_val));
    BOOST_TEST(isinf(inf_val + one));
    BOOST_TEST(isinf(one + inf_val));
    BOOST_TEST(isnan(inf_val + qnan_val));
    BOOST_TEST(isnan(qnan_val + inf_val));

    // Overflow
    constexpr decimal32 max_val((std::numeric_limits<decimal32>::max)());
    BOOST_TEST(isinf(max_val + one));
}

int main()
{
    test_comp();
    test_binary_constructor();
    test_decimal_constructor();
    test_non_finite_values();
    test_unary_arithmetic();

    test_addition();

    return boost::report_errors();
}
