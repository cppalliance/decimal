// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"

#include <boost/decimal.hpp>
#include <bitset>
#include <limits>
#include <random>
#include <cmath>
#include <cerrno>

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <boost/core/lightweight_test.hpp>

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

    decimal32 zero {0, 0};
    decimal32 one {1, 0};
    decimal32 half {5, -1};
    BOOST_TEST(zero < one);
    BOOST_TEST(zero < half);
    BOOST_TEST(one > zero);
    BOOST_TEST(half > zero);
    BOOST_TEST(zero > -one);
    BOOST_TEST(half > -one);
    BOOST_TEST(-one < zero);
    BOOST_TEST(-one < half);

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

    BOOST_TEST(small <= std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(small >= -std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(!(small <= std::numeric_limits<decimal32>::signaling_NaN()));
    BOOST_TEST(!(small <= std::numeric_limits<decimal32>::quiet_NaN()));
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

    #ifdef _MSC_VER

    BOOST_TEST(boost::decimal::isfinite(one));
    BOOST_TEST(boost::decimal::isfinite(std::numeric_limits<decimal32>::denorm_min()));
    BOOST_TEST(!boost::decimal::isfinite(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!boost::decimal::isfinite(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(!boost::decimal::isfinite(std::numeric_limits<decimal32>::signaling_NaN()));

    #else

    BOOST_TEST(isfinite(one));
    BOOST_TEST(isfinite(std::numeric_limits<decimal32>::denorm_min()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::infinity()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::quiet_NaN()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal32>::signaling_NaN()));

    #endif

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

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<std::uint32_t> dist(1, 2);

    BOOST_TEST(isnan(detail::check_non_finite(one, std::numeric_limits<decimal32>::quiet_NaN() * dist(rng))));
    BOOST_TEST(isnan(detail::check_non_finite(std::numeric_limits<decimal32>::quiet_NaN() * dist(rng), one)));
    BOOST_TEST(isinf(detail::check_non_finite(one, std::numeric_limits<decimal32>::infinity() * dist(rng))));
    BOOST_TEST(isinf(detail::check_non_finite(std::numeric_limits<decimal32>::infinity() * dist(rng), one)));
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
    constexpr decimal32 full_length_num {1000000, 1};
    constexpr decimal32 rounded_full_length_num(1000001, 1);
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
    if (!BOOST_TEST(isinf(max_val + one)))
    {
        // LCOV_EXCL_START
        std::cerr << std::bitset<32>(to_bits(max_val + one)) << std::endl;
        // LCOV_EXCL_STOP
    }
}

void test_subtraction()
{
    // Case 1: The difference is more than the digits of accuracy
    constexpr decimal32 big_num(0b1, 20);
    constexpr decimal32 small_num(0b1, -20);
    BOOST_TEST_EQ(big_num - small_num, big_num);
    BOOST_TEST_EQ(small_num - big_num, -big_num);

    // Case 2: Round the last digit of the significand
    constexpr decimal32 no_round {1234567, 5};
    constexpr decimal32 round {9876543, -2};
    BOOST_TEST_EQ(no_round - round, decimal32(1234566, 5));

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

    constexpr decimal32 max(9'999'999, 0);
    constexpr decimal32 max_plus_one(10'000'000, 0);
    BOOST_TEST_EQ(max_plus_one - one, max);

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
    #ifdef _MSC_VER
    #  pragma warning(push)
    #  pragma warning(disable: 4146)
    #endif

    // Underflow
    constexpr decimal32 lowest_val(std::numeric_limits<decimal32>::lowest());
    BOOST_TEST(isinf(lowest_val - one));

    #ifdef _MSC_VER
    #  pragma warning(pop)
    #endif
}

void test_multiplicatiom()
{
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 one {1, 0};
    constexpr decimal32 two {2, 0};
    constexpr decimal32 four {4, 0};
    constexpr decimal32 eight {8, 0};

    BOOST_TEST_EQ(zero * one, zero);
    BOOST_TEST_EQ(zero * -one, zero);
    BOOST_TEST_EQ(one * two, two);

    decimal32 pow_two {1, 0};
    BOOST_TEST_EQ(pow_two *= two, two);
    BOOST_TEST_EQ(pow_two *= two, four);
    BOOST_TEST_EQ(pow_two *= -two, -eight);

    // Non-finite values
    constexpr decimal32 qnan_val(std::numeric_limits<decimal32>::quiet_NaN());
    constexpr decimal32 snan_val(std::numeric_limits<decimal32>::signaling_NaN());
    constexpr decimal32 inf_val(std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(isnan(qnan_val * one));
    BOOST_TEST(isnan(snan_val * one));
    BOOST_TEST(isnan(one * qnan_val));
    BOOST_TEST(isnan(one * snan_val));
    BOOST_TEST(isinf(inf_val * one));
    BOOST_TEST(isinf(one * inf_val));
    BOOST_TEST(isnan(inf_val * qnan_val));
    BOOST_TEST(isnan(qnan_val * inf_val));
}

void test_div_mod()
{
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 one {1, 0};
    constexpr decimal32 two {2, 0};
    constexpr decimal32 three {3, 0};
    constexpr decimal32 four {4, 0};
    constexpr decimal32 eight {8, 0};
    constexpr decimal32 half {5, -1};
    constexpr decimal32 quarter {25, -2};
    constexpr decimal32 eighth {125, -3};

    BOOST_TEST_EQ(two / one, two);
    BOOST_TEST_EQ(two % one, zero);
    BOOST_TEST_EQ(eight / four, two);
    BOOST_TEST_EQ(four / eight, half);
    BOOST_TEST_EQ(one / four, quarter);
    BOOST_TEST_EQ(one / eight, eighth);
    BOOST_TEST_EQ(three / two, one + half);

    // From https://en.cppreference.com/w/cpp/numeric/math/fmod
    BOOST_TEST_EQ(decimal32(51, -1) % decimal32(30, -1), decimal32(21, -1));

    // Non-finite values
    constexpr decimal32 qnan_val(std::numeric_limits<decimal32>::quiet_NaN());
    constexpr decimal32 snan_val(std::numeric_limits<decimal32>::signaling_NaN());
    constexpr decimal32 inf_val(std::numeric_limits<decimal32>::infinity());
    BOOST_TEST(isnan(qnan_val / one));
    BOOST_TEST(isnan(snan_val / one));
    BOOST_TEST(isnan(one / qnan_val));
    BOOST_TEST(isnan(one / snan_val));
    BOOST_TEST(isinf(inf_val / one));
    BOOST_TEST_EQ(one / inf_val, zero);
    BOOST_TEST(isnan(inf_val / qnan_val));
    BOOST_TEST(isnan(qnan_val / inf_val));

    // Mixed types
    BOOST_TEST(isnan(qnan_val / 1));
    BOOST_TEST(isnan(snan_val / 1));
    BOOST_TEST(isnan(1 / qnan_val));
    BOOST_TEST(isnan(1 / snan_val));
    BOOST_TEST(isinf(inf_val / 1));
    BOOST_TEST_EQ(1 / inf_val, zero);
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

    // Check the edge cases for use of the combination field
    constexpr decimal32 edge_20 (detail::d32_no_combination);
    BOOST_TEST_EQ(static_cast<T>(edge_20), detail::d32_no_combination);

    constexpr decimal32 edge_23 (detail::d32_big_combination);
    BOOST_TEST_EQ(static_cast<T>(edge_23), detail::d32_big_combination);

    constexpr decimal32 max_sig (detail::max_significand);
    if (!BOOST_TEST_EQ(static_cast<T>(max_sig), detail::max_significand))
    {
        // LCOV_EXCL_START
        std::cerr << "Bits: " << std::bitset<32>(to_bits(max_sig)) << std::endl;
        // LCOV_EXCL_STOP
    }
}

template <typename T>
void test_construct_from_float()
{
    constexpr decimal32 one(1, 0);
    decimal32 float_one(T(1));
    if(!BOOST_TEST_EQ(one, float_one))
    {
        debug_pattern(float_one);
    }

    constexpr decimal32 fraction(12345, -4);
    decimal32 float_frac(T(1.2345));
    if(!BOOST_TEST_EQ(fraction, float_frac))
    {
        debug_pattern(float_frac);
    }

    constexpr decimal32 neg_frac(98123, -4, true);
    decimal32 neg_float_frac(T(-9.8123));
    if(!BOOST_TEST_EQ(neg_frac, neg_float_frac))
    {
        debug_pattern(neg_float_frac);
    }
}

template <typename T>
void spot_check_addition(T a, T b, T res)
{
    decimal32 dec_a {a};
    decimal32 dec_b {b};
    decimal32 dec_res {res};

    if (!BOOST_TEST_EQ(dec_a + dec_b, dec_res))
    {
        // LCOV_EXCL_START
        std::cerr << "A + B: " << a + b
                  << "\nIn dec: " << decimal32(a + b) << std::endl;
        // LCOV_EXCL_STOP
    }
}

void test_hash()
{
    decimal32 one {1, 0};
    decimal32 zero {0, 0};

    BOOST_TEST_NE(std::hash<decimal32>{}(one), std::hash<decimal32>{}(zero));
}

void test_shrink_significand()
{
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<std::uint64_t> dist(100'000'000'000, 100'000'000'000);
    std::int32_t pow {};
    std::uint64_t sig {dist(rng)};

    detail::shrink_significand(sig, pow);
    BOOST_TEST_EQ(pow, 3);
}

int main()
{
    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    test_binary_constructor();
    #endif

    test_decimal_constructor();
    test_non_finite_values();
    test_unary_arithmetic();

    test_construct_from_integer<int>();
    test_construct_from_integer<long>();
    test_construct_from_integer<long long>();

    test_construct_from_float<float>();
    test_construct_from_float<double>();
    test_construct_from_float<long double>();
    #ifdef BOOST_DECIMAL_HAS_FLOAT128
    test_construct_from_float<__float128>();
    #endif

    test_comp();

    test_addition();
    test_subtraction();
    test_multiplicatiom();
    test_div_mod();

    test_hash();

    spot_check_addition(-1054191000, -920209700, -1974400700);
    spot_check_addition(353582500, -32044770, 321537730);
    spot_check_addition(989629100, 58451350, 1048080000);

    test_shrink_significand();

    return boost::report_errors();
}
