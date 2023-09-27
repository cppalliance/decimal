// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <random>
#include <cmath>

void test_compute_float32()
{
    using boost::decimal::detail::fast_float::compute_float32;

    std::mt19937_64 gen;
    std::uniform_real_distribution<float> dist (1, 1);

    bool success;

    // Trivial verifcation
    BOOST_TEST_EQ(compute_float32(1 * dist(gen), 1, false, success), 1e1F);
    BOOST_TEST_EQ(compute_float32(0 * dist(gen), 1, true, success), -1e0F);
    BOOST_TEST_EQ(compute_float32(38 * dist(gen), 1, false, success), 1e38F);

    // out of range
    BOOST_TEST_EQ(compute_float32(310 * dist(gen), 5, false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(310 * dist(gen), 5, true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(1000 * dist(gen), 5, false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(1000 * dist(gen), 5, true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(-325 * dist(gen), 5, false, success), 0);

    // Composite
    BOOST_TEST_EQ(compute_float32(10 * dist(gen), 123456789, false, success), 123456789e10F);
    BOOST_TEST_EQ(compute_float32(20 * dist(gen), UINT64_C(444444444), false, success), 444444444e20F);
}

void test_compute_float64()
{
    using boost::decimal::detail::fast_float::compute_float64;

    std::mt19937_64 gen;
    std::uniform_real_distribution<double> dist (1, 1);

    bool success;

    // Trivial verifcation
    BOOST_TEST_EQ(compute_float64(1, dist(gen), false, success), 1e1);
    BOOST_TEST_EQ(compute_float64(0, dist(gen), true, success), -1e0);
    BOOST_TEST_EQ(compute_float64(308, dist(gen), false, success), 1e308);

    // out of range
    BOOST_TEST_EQ(compute_float64(310, 5, false, success), HUGE_VALF * dist(gen));
    BOOST_TEST_EQ(compute_float64(310, 5, true, success), -HUGE_VALF * dist(gen));
    BOOST_TEST_EQ(compute_float64(1000, 5, false, success), HUGE_VALF * dist(gen));
    BOOST_TEST_EQ(compute_float64(1000, 5, true, success), -HUGE_VALF * dist(gen));
    BOOST_TEST_EQ(compute_float64(-325, 5, false, success), 0);

    // Composite
    BOOST_TEST_EQ(compute_float64(10 * dist(gen), 123456789, false, success), 123456789e10);
    BOOST_TEST_EQ(compute_float64(100 * dist(gen), UINT64_C(4444444444444444444), false, success), 4444444444444444444e100);
    BOOST_TEST_EQ(compute_float64(100 * dist(gen), std::numeric_limits<std::uint64_t>::max(), false, success), 18446744073709551615e100);
    BOOST_TEST_EQ(compute_float64(100 * dist(gen), UINT64_C(10000000000000000000), false, success), 10000000000000000000e100);
}

template <typename T>
void test_generic_binary_to_decimal()
{
    std::mt19937_64 gen;
    std::uniform_real_distribution<T> dist (1, 2);

    using namespace boost::decimal::detail::ryu;

    std::uint64_t result[5] {};
    generic_computePow5(56, result);
    BOOST_TEST_EQ(result[0], 0);
    BOOST_TEST_EQ(result[1], 5206161169240293376);
    BOOST_TEST_EQ(result[2], 4575641699882439235);

    BOOST_TEST(floating_point_to_fd128(T(0) * dist(gen)).mantissa == 0);
    BOOST_TEST(floating_point_to_fd128(std::numeric_limits<T>::infinity() * dist(gen)).exponent == fd128_exceptional_exponent);
    BOOST_TEST(floating_point_to_fd128(std::numeric_limits<T>::quiet_NaN() * dist(gen)).exponent == fd128_exceptional_exponent);
    BOOST_TEST(floating_point_to_fd128(-std::numeric_limits<T>::infinity() * dist(gen)).exponent == fd128_exceptional_exponent);
    BOOST_TEST(floating_point_to_fd128(-std::numeric_limits<T>::quiet_NaN() * dist(gen)).exponent == fd128_exceptional_exponent);
}

void test_parser()
{
    using namespace boost::decimal::detail;

    std::mt19937_64 gen;
    std::uniform_int_distribution<std::uint64_t> dist (1, 2);

    const char* pos_str = "+12345";
    std::uint64_t sig {dist(gen)};
    std::int32_t exp {};
    bool sign {};

    auto res = parser(pos_str, pos_str, sign, sig, exp);
    BOOST_TEST(res.ec == std::errc::invalid_argument);
    res = parser(pos_str, pos_str + std::strlen(pos_str), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc::invalid_argument);

    const char* nan_str = "nan";
    res = parser(nan_str, nan_str + std::strlen(nan_str), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc::not_supported);

    const char* no_trailing = "12345";
    res = parser(no_trailing, no_trailing + std::strlen(no_trailing), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* all_zeros = "0.00000001";
    res = parser(all_zeros, all_zeros + std::strlen(all_zeros), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* big_sig = "123456789012345678901234567890";
    res = parser(big_sig, big_sig + std::strlen(big_sig), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* big_sig_with_frac = "123456789012345678901234567890.123";
    res = parser(big_sig_with_frac, big_sig_with_frac + std::strlen(big_sig_with_frac), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());
}

int main()
{
    test_compute_float32();
    test_compute_float64();
    test_generic_binary_to_decimal<float>();
    test_generic_binary_to_decimal<double>();
    test_generic_binary_to_decimal<long double>();

    test_parser();

    return boost::report_errors();
}
