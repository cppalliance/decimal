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

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wundef"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wsign-conversion"
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

void test_comp()
{
    constexpr decimal64_fast small(1, -50);

    BOOST_TEST(small == small);

    constexpr decimal64_fast sig(123456, -50);
    BOOST_TEST(sig != small);

    BOOST_TEST(small < sig);
    BOOST_TEST(small <= sig);
    BOOST_TEST(small <= small);
    BOOST_TEST(sig > small);
    BOOST_TEST(sig >= small);

    decimal64_fast zero {0, 0};
    decimal64_fast one {1, 0};
    decimal64_fast half {5, -1};
    BOOST_TEST(zero < one);
    BOOST_TEST(zero < half);
    BOOST_TEST(one > zero);
    BOOST_TEST(half > zero);
    BOOST_TEST(zero > -one);
    BOOST_TEST(half > -one);
    BOOST_TEST(-one < zero);
    BOOST_TEST(-one < half);

    // Test cohorts
    BOOST_TEST(small == decimal64_fast(10, -51));
    BOOST_TEST(small == decimal64_fast(100, -52));
    BOOST_TEST(small == decimal64_fast(1000, -53));
    BOOST_TEST(small == decimal64_fast(10000, -54));
    BOOST_TEST(small == decimal64_fast(100000, -55));
    BOOST_TEST(small == decimal64_fast(1000000, -56));

    // Test non-finite comp
    BOOST_TEST(small < std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(small > -std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(!(small == std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(small != std::numeric_limits<decimal64_fast>::infinity());

    BOOST_TEST(!(small < std::numeric_limits<decimal64_fast>::signaling_NaN()));
    BOOST_TEST(!(small < std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(small != std::numeric_limits<decimal64_fast>::quiet_NaN());
    BOOST_TEST(std::numeric_limits<decimal64_fast>::quiet_NaN() != std::numeric_limits<decimal64_fast>::quiet_NaN());

    BOOST_TEST(small <= std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(small >= -std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(!(small <= std::numeric_limits<decimal64_fast>::signaling_NaN()));
    BOOST_TEST(!(small <= std::numeric_limits<decimal64_fast>::quiet_NaN()));
}

void test_non_finite_values()
{
    constexpr decimal64_fast one(0b1, 0);

    BOOST_TEST(std::numeric_limits<decimal64_fast>::has_infinity);
    BOOST_TEST(isinf(std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(isinf(-std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!isinf(one));
    BOOST_TEST(!isinf(std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!isinf(std::numeric_limits<decimal64_fast>::signaling_NaN()));

    BOOST_TEST(std::numeric_limits<decimal64_fast>::has_quiet_NaN);
    BOOST_TEST(std::numeric_limits<decimal64_fast>::has_signaling_NaN);
    BOOST_TEST(isnan(std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(isnan(std::numeric_limits<decimal64_fast>::signaling_NaN()));
    BOOST_TEST(!isnan(one));
    BOOST_TEST(!isnan(std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!isnan(-std::numeric_limits<decimal64_fast>::infinity()));

    BOOST_TEST(!issignaling(std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(issignaling(std::numeric_limits<decimal64_fast>::signaling_NaN()));
    BOOST_TEST(!issignaling(one));
    BOOST_TEST(!issignaling(std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!issignaling(-std::numeric_limits<decimal64_fast>::infinity()));

    #ifdef _MSC_VER

    BOOST_TEST(boost::decimal::isfinite(one));
    BOOST_TEST(!boost::decimal::isfinite(std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!boost::decimal::isfinite(std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!boost::decimal::isfinite(std::numeric_limits<decimal64_fast>::signaling_NaN()));

    #else

    BOOST_TEST(isfinite(one));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!isfinite(std::numeric_limits<decimal64_fast>::signaling_NaN()));

    #endif

    BOOST_TEST(isnormal(one));
    BOOST_TEST(!isnormal(std::numeric_limits<decimal64_fast>::infinity()));
    BOOST_TEST(!isnormal(std::numeric_limits<decimal64_fast>::quiet_NaN()));
    BOOST_TEST(!isnormal(std::numeric_limits<decimal64_fast>::signaling_NaN()));

    BOOST_TEST_EQ(fpclassify(one), FP_NORMAL);
    BOOST_TEST_EQ(fpclassify(-one), FP_NORMAL);
    BOOST_TEST_EQ(fpclassify(std::numeric_limits<decimal64_fast>::quiet_NaN()), FP_NAN);
    BOOST_TEST_EQ(fpclassify(std::numeric_limits<decimal64_fast>::signaling_NaN()), FP_NAN);
    BOOST_TEST_EQ(fpclassify(std::numeric_limits<decimal64_fast>::infinity()), FP_INFINITE);
    BOOST_TEST_EQ(fpclassify(-std::numeric_limits<decimal64_fast>::infinity()), FP_INFINITE);

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<std::uint32_t> dist(1, 2);

    BOOST_TEST(isnan(detail::check_non_finite(one, std::numeric_limits<decimal64_fast>::quiet_NaN() * dist(rng))));
    BOOST_TEST(isnan(detail::check_non_finite(std::numeric_limits<decimal64_fast>::quiet_NaN() * dist(rng), one)));
    BOOST_TEST(isinf(detail::check_non_finite(one, std::numeric_limits<decimal64_fast>::infinity() * dist(rng))));
    BOOST_TEST(isinf(detail::check_non_finite(std::numeric_limits<decimal64_fast>::infinity() * dist(rng), one)));
}

#if !defined(__GNUC__) || (__GNUC__ != 7 && __GNUC__ != 8)
void test_unary_arithmetic()
{
    constexpr decimal64_fast one(1);
    BOOST_TEST(+one == one);
    if(!BOOST_TEST(-one != one))
    {
        // LCOV_EXCL_START
        std::cerr << "One: " << one
                  << "\nNeg: " << -one
                  << "\n    Bid: " << to_bid(one)
                  << "\nNeg Bid: " << to_bid(-one) << std::endl;
        // LCOV_EXCL_STOP
    }
}
#endif

void test_addition()
{
    // Case 1: The difference is more than the digits of accuracy
    constexpr decimal64_fast big_num(0b1, 20);
    constexpr decimal64_fast small_num(0b1, -20);
    BOOST_TEST_EQ(big_num + small_num, big_num);
    BOOST_TEST_EQ(small_num + big_num, big_num);

    // Case 2: Round the last digit of the significand
    constexpr decimal64_fast full_length_num {UINT64_C(1000000000000000), 0};
    constexpr decimal64_fast rounded_full_length_num(UINT64_C(1000000000000001), 0);
    constexpr decimal64_fast no_round(1, -1);
    constexpr decimal64_fast round(9, -1);
    BOOST_TEST_EQ(full_length_num + no_round, full_length_num);
    BOOST_TEST_EQ(full_length_num + round, rounded_full_length_num);

    // Case 3: Add away
    constexpr decimal64_fast one(1, 0);
    constexpr decimal64_fast two(2, 0);
    constexpr decimal64_fast three(3, 0);
    decimal64_fast mutable_one(1, 0);

    BOOST_TEST_EQ(one + one, two);
    BOOST_TEST_EQ(two + one, three);
    BOOST_TEST_EQ(one + one + one, three);

    // Pre- and post- increment
    BOOST_TEST_EQ(mutable_one, one);
    BOOST_TEST_EQ(mutable_one++, two);
    BOOST_TEST_EQ(++mutable_one, three);

    // Different orders of magnitude
    constexpr decimal64_fast ten(10, 0);
    constexpr decimal64_fast eleven(11, 0);
    BOOST_TEST_EQ(ten + one, eleven);

    constexpr decimal64_fast max_sig(9'999'999, 0);
    constexpr decimal64_fast max_plus_one(10'000'000, 0);
    BOOST_TEST_EQ(max_sig + one, max_plus_one);

    // Non-finite values
    constexpr decimal64_fast qnan_val(std::numeric_limits<decimal64_fast>::quiet_NaN());
    constexpr decimal64_fast snan_val(std::numeric_limits<decimal64_fast>::signaling_NaN());
    constexpr decimal64_fast inf_val(std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(isnan(qnan_val + one));
    BOOST_TEST(isnan(snan_val + one));
    BOOST_TEST(isnan(one + qnan_val));
    BOOST_TEST(isnan(one + snan_val));
    BOOST_TEST(isinf(inf_val + one));
    BOOST_TEST(isinf(one + inf_val));
    BOOST_TEST(isnan(inf_val + qnan_val));
    BOOST_TEST(isnan(qnan_val + inf_val));
}

void test_subtraction()
{
    // Case 1: The difference is more than the digits of accuracy
    constexpr decimal64_fast big_num(0b1, 20);
    constexpr decimal64_fast small_num(0b1, -20);
    BOOST_TEST_EQ(big_num - small_num, big_num);
    BOOST_TEST_EQ(small_num - big_num, -big_num);

    constexpr decimal64_fast one(1, 0);
    constexpr decimal64_fast two(2, 0);
    constexpr decimal64_fast three(3, 0);
    decimal64_fast mutable_three(3, 0);

    BOOST_TEST_EQ(two - one, one);
    BOOST_TEST_EQ(three - one - one, one);

    // Pre- and post- increment
    BOOST_TEST_EQ(mutable_three, three);
    BOOST_TEST_EQ(mutable_three--, two);
    BOOST_TEST_EQ(--mutable_three, one);

    // Different orders of magnitude
    constexpr decimal64_fast ten(10, 0);
    constexpr decimal64_fast eleven(11, 0);
    BOOST_TEST_EQ(eleven - one, ten);

    constexpr decimal64_fast max(9'999'999, 0);
    constexpr decimal64_fast max_plus_one(10'000'000, 0);
    BOOST_TEST_EQ(max_plus_one - one, max);

    // Non-finite values
    constexpr decimal64_fast qnan_val(std::numeric_limits<decimal64_fast>::quiet_NaN());
    constexpr decimal64_fast snan_val(std::numeric_limits<decimal64_fast>::signaling_NaN());
    constexpr decimal64_fast inf_val(std::numeric_limits<decimal64_fast>::infinity());
    BOOST_TEST(isnan(qnan_val - one));
    BOOST_TEST(isnan(snan_val - one));
    BOOST_TEST(isnan(one - qnan_val));
    BOOST_TEST(isnan(one - snan_val));
    BOOST_TEST(isinf(inf_val - one));
    BOOST_TEST(isinf(one - inf_val));
    BOOST_TEST(isnan(inf_val - qnan_val));
    BOOST_TEST(isnan(qnan_val - inf_val));
}

void test_multiplicatiom()
{
    constexpr decimal64_fast zero {0, 0};
    constexpr decimal64_fast one {1, 0};
    constexpr decimal64_fast two {2, 0};
    constexpr decimal64_fast four {4, 0};
    constexpr decimal64_fast eight {8, 0};

    BOOST_TEST_EQ(zero * one, zero);
    BOOST_TEST_EQ(zero * -one, -zero);
    BOOST_TEST_EQ(one * two, two);

    decimal64_fast pow_two {1, 0};
    BOOST_TEST_EQ(pow_two *= two, two);
    BOOST_TEST_EQ(pow_two *= two, four);
    BOOST_TEST_EQ(pow_two *= -two, -eight);

    // Non-finite values
    constexpr decimal64_fast qnan_val(std::numeric_limits<decimal64_fast>::quiet_NaN());
    constexpr decimal64_fast snan_val(std::numeric_limits<decimal64_fast>::signaling_NaN());
    constexpr decimal64_fast inf_val(std::numeric_limits<decimal64_fast>::infinity());
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
    constexpr decimal64_fast zero {0, 0};
    constexpr decimal64_fast one {1, 0};
    constexpr decimal64_fast two {2, 0};
    constexpr decimal64_fast three {3, 0};
    constexpr decimal64_fast four {4, 0};
    constexpr decimal64_fast eight {8, 0};
    constexpr decimal64_fast half {5, -1};
    constexpr decimal64_fast quarter {25, -2};
    constexpr decimal64_fast eighth {125, -3};

    BOOST_TEST_EQ(two / one, two);
    BOOST_TEST_EQ(two % one, zero);
    BOOST_TEST_EQ(eight / four, two);
    BOOST_TEST_EQ(four / eight, half);
    BOOST_TEST_EQ(one / four, quarter);
    BOOST_TEST_EQ(one / eight, eighth);
    BOOST_TEST_EQ(three / two, one + half);

    // From https://en.cppreference.com/w/cpp/numeric/math/fmod
    BOOST_TEST_EQ(decimal64_fast(51, -1) % decimal64_fast(30, -1), decimal64_fast(21, -1));

    // Non-finite values
    constexpr decimal64_fast qnan_val(std::numeric_limits<decimal64_fast>::quiet_NaN());
    constexpr decimal64_fast snan_val(std::numeric_limits<decimal64_fast>::signaling_NaN());
    constexpr decimal64_fast inf_val(std::numeric_limits<decimal64_fast>::infinity());
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
    constexpr decimal64_fast one(1, 0);
    BOOST_TEST_EQ(one, decimal64_fast(T(1)));

    constexpr decimal64_fast one_pow_eight(1, 8);
    BOOST_TEST_EQ(one_pow_eight, decimal64_fast(T(100'000'000)));
}

template <typename T>
void test_construct_from_float()
{
    constexpr decimal64_fast one(1, 0);
    decimal64_fast float_one(T(1));
    BOOST_TEST_EQ(one, float_one);

    constexpr decimal64_fast fraction(12345, -4);
    decimal64_fast float_frac(T(1.2345));
    BOOST_TEST_EQ(fraction, float_frac);

    constexpr decimal64_fast neg_frac(-98123, -4);
    decimal64_fast neg_float_frac(T(-9.8123));
    BOOST_TEST_EQ(neg_frac, neg_float_frac);
}

template <typename T>
void spot_check_addition(T a, T b, T res)
{
    decimal64_fast dec_a {a};
    decimal64_fast dec_b {b};
    decimal64_fast dec_res {res};

    if (!BOOST_TEST_EQ(dec_a + dec_b, dec_res))
    {
        // LCOV_EXCL_START
        std::cerr << "A + B: " << a + b
                  << "\nIn dec: " << decimal64_fast(a + b) << std::endl;
        // LCOV_EXCL_STOP
    }
}

void test_hash()
{
    decimal64_fast one {1, 0};
    decimal64_fast zero {0, 0};

    BOOST_TEST_NE(std::hash<decimal64_fast>{}(one), std::hash<decimal64_fast>{}(zero));
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
    test_non_finite_values();

    #if !defined(__GNUC__) || (__GNUC__ != 7 && __GNUC__ != 8)
    test_unary_arithmetic();
    #endif

    test_construct_from_integer<int>();
    test_construct_from_integer<long>();
    test_construct_from_integer<long long>();

    test_construct_from_float<float>();
    test_construct_from_float<double>();
    test_construct_from_float<long double>();
    #if defined(BOOST_DECIMAL_HAS_FLOAT128) && (!defined(__clang_major__) || __clang_major__ >= 13)
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
    spot_check_addition(989629100, 58451350, 1048080450);

    test_shrink_significand();

    return boost::report_errors();
}
