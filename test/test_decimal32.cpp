// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <bitset>
#include <limits>
#include <cmath>
#include <cerrno>

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

#if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
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

    // Construct denorm min
    decimal32 denorm_min(0b1, -101);
    // 0 for sign
    // 00000 for combination field
    // 000000 for exp
    // 0000000000'00000000001
    BOOST_TEST_EQ(std::bitset<32>(to_bits(denorm_min)), std::bitset<32>(0b0'00000'000000'0000000000'0000000001));
}
#endif

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
    constexpr decimal32 one(0b1, 0);

    BOOST_TEST(std::numeric_limits<decimal32>::has_infinity);
    BOOST_TEST(isinf(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(isinf(-std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!isinf(one));
    BOOST_TEST(!isinf(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(!isinf(std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!isinf(std::numeric_limits<decimal32>::denorm_min()));

    BOOST_TEST(std::numeric_limits<decimal32>::has_quiet_NaN);
    BOOST_TEST(std::numeric_limits<decimal32>::has_signaling_NaN);
    BOOST_TEST(isnan(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(isnan(std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!isnan(one));
    BOOST_TEST(!isnan(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!isnan(-std::numeric_limits<decimal32>::infinity()));

    BOOST_TEST(!issignaling(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(issignaling(std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!issignaling(one));
    BOOST_TEST(!issignaling(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!issignaling(-std::numeric_limits<decimal32>::infinity()));

    BOOST_TEST(isfinite(one));
    BOOST_TEST(isfinite(std::numeric_limits<decimal32>::denorm_min()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::signaling_NaN()));

    BOOST_TEST(isnormal(one));
    BOOST_TEST(!isnormal(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!isnormal(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(!isnormal(std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!isnormal(std::numeric_limits<decimal32>::denorm_min()));

    BOOST_TEST_EQ(fpclassify(one), FP_NORMAL);
    BOOST_TEST_EQ(fpclassify(-one), FP_NORMAL);
    BOOST_TEST_EQ(fpclassify(std::numeric_limits<decimal32>::quiet_NaN()), FP_NAN);
    BOOST_TEST_EQ(fpclassify(std::numeric_limits<decimal32>::signaling_NaN()), FP_NAN);
    BOOST_TEST_EQ(fpclassify(std::numeric_limits<decimal32>::infinity()), FP_INFINITE);
    BOOST_TEST_EQ(fpclassify(-std::numeric_limits<decimal32>::infinity()), FP_INFINITE);
    BOOST_TEST_EQ(fpclassify(std::numeric_limits<decimal32>::denorm_min()), FP_SUBNORMAL);
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
    decimal32 mutable_one(1, 0);

    BOOST_TEST_EQ(one + one, two);
    BOOST_TEST_EQ(two + one, three);
    BOOST_TEST_EQ(one + one + one, three);

    // Pre- and post- increment
    BOOST_TEST_EQ(mutable_one, one);
    BOOST_TEST_EQ(mutable_one++, two);
    BOOST_TEST_EQ(++mutable_one, three);

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

void test_subtraction()
{
    // Case 1: The difference is more than the digits of accuracy
    constexpr decimal32 big_num(0b1, 20);
    constexpr decimal32 small_num(0b1, -20);
    BOOST_TEST_EQ(big_num - small_num, big_num);
    BOOST_TEST_EQ(small_num - big_num, big_num);

    // Case 2: Round the last digit of the significand
    constexpr decimal32 no_round {1234567, 5};
    constexpr decimal32 round {9876543, -2};
    BOOST_TEST_EQ(no_round - round, no_round);

    // Case 3: Add away
    constexpr decimal32 one(1, 0);
    constexpr decimal32 two(2, 0);
    constexpr decimal32 three(3, 0);
    decimal32 mutable_three(3, 0);

    BOOST_TEST_EQ(two - one, one);
    BOOST_TEST_EQ(three - one - one, one);

    // Pre- and post- increment
    BOOST_TEST_EQ(mutable_three, three);
    BOOST_TEST_EQ(mutable_three--, two);
    BOOST_TEST_EQ(--mutable_three, one);

    // Different orders of magnitude
    constexpr decimal32 ten(10, 0);
    constexpr decimal32 eleven(11, 0);
    BOOST_TEST_EQ(eleven - one, ten);

    // Too great a difference for one to matter
    constexpr decimal32 max_plus_one(10'000'000, 0);
    BOOST_TEST_EQ(max_plus_one - one, max_plus_one);

    // Non-finite values
    constexpr decimal32 qnan_val(std::numeric_limits<decimal32>::quiet_NaN());
    constexpr decimal32 snan_val(std::numeric_limits<decimal32>::signaling_NaN());
    constexpr decimal32 inf_val(std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(isnan(qnan_val - one));
    BOOST_TEST(isnan(snan_val - one));
    BOOST_TEST(isnan(one - qnan_val));
    BOOST_TEST(isnan(one - snan_val));
    BOOST_TEST(isinf(inf_val - one));
    BOOST_TEST(isinf(one - inf_val));
    BOOST_TEST(isnan(inf_val - qnan_val));
    BOOST_TEST(isnan(qnan_val - inf_val));

    // Why does MSVC 14.1 warn about unary minus but nothing else does?
    #ifdef BOOST_MSVC
    #  pragma warning(push)
    #  pragma warning(disable: 4146)
    #endif

    // Underflow
    constexpr decimal32 lowest_val(std::numeric_limits<decimal32>::lowest());
    BOOST_TEST(isinf(lowest_val - one));

    #ifdef BOOST_MSVC
    #  pragma warning(pop)
    #endif
}

template <typename T>
void test_construct_from_integer()
{
    constexpr decimal32 one(1, 0);
    BOOST_TEST_EQ(one, decimal32(T(1)));

    constexpr decimal32 one_pow_eight(1, 8);
    BOOST_TEST_EQ(one_pow_eight, decimal32(T(100'000'000)));

    constexpr decimal32 rounded(1234568, 1);
    BOOST_TEST_EQ(rounded, decimal32(T(12345678)));
}

int main()
{
    test_comp();

    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    test_binary_constructor();
    #endif

    test_decimal_constructor();
    test_non_finite_values();
    test_unary_arithmetic();

    test_addition();
    test_subtraction();

    test_construct_from_integer<int>();
    test_construct_from_integer<long>();
    test_construct_from_integer<long long>();

    return boost::report_errors();
}
