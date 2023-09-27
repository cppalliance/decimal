// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <cmath>

void test_compute_float32()
{
    using boost::decimal::detail::fast_float::compute_float32;

    bool success;

    // Trivial verifcation
    BOOST_TEST_EQ(compute_float32(1, 1, false, success), 1e1F);
    BOOST_TEST_EQ(compute_float32(0, 1, true, success), -1e0F);
    BOOST_TEST_EQ(compute_float32(38, 1, false, success), 1e38F);

    // out of range
    BOOST_TEST_EQ(compute_float32(310, 5, false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(310, 5, true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(1000, 5, false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(1000, 5, true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float32(-325, 5, false, success), 0);

    // Composite
    BOOST_TEST_EQ(compute_float32(10, 123456789, false, success), 123456789e10F);
    BOOST_TEST_EQ(compute_float32(20, UINT64_C(444444444), false, success), 444444444e20F);
}

void test_compute_float64()
{
    using boost::decimal::detail::fast_float::compute_float64;

    bool success;

    // Trivial verifcation
    BOOST_TEST_EQ(compute_float64(1, 1, false, success), 1e1);
    BOOST_TEST_EQ(compute_float64(0, 1, true, success), -1e0);
    BOOST_TEST_EQ(compute_float64(308, 1, false, success), 1e308);

    // out of range
    BOOST_TEST_EQ(compute_float64(310, 5, false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(310, 5, true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(1000, 5, false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(1000, 5, true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(-325, 5, false, success), 0);

    // Composite
    BOOST_TEST_EQ(compute_float64(10, 123456789, false, success), 123456789e10);
    BOOST_TEST_EQ(compute_float64(100, UINT64_C(4444444444444444444), false, success), 4444444444444444444e100);
    BOOST_TEST_EQ(compute_float64(100, std::numeric_limits<std::uint64_t>::max(), false, success), 18446744073709551615e100);
    BOOST_TEST_EQ(compute_float64(100, UINT64_C(10000000000000000000), false, success), 10000000000000000000e100);
}

template <typename T>
void test_generic_binary_to_decimal()
{
    using namespace boost::decimal::detail::ryu;

    std::uint64_t* result;
    result = (std::uint64_t*)malloc(4);
    generic_computePow5(56, result);
    BOOST_TEST_EQ(result[0], 0);
    BOOST_TEST_EQ(result[1], 5206161169240293376);
    BOOST_TEST_EQ(result[2], 4575641699882439235);
    free(result);

    BOOST_TEST(floating_point_to_fd128(T(0)).mantissa == 0);
    BOOST_TEST(floating_point_to_fd128(std::numeric_limits<T>::infinity()).exponent == fd128_exceptional_exponent);
    BOOST_TEST(floating_point_to_fd128(std::numeric_limits<T>::quiet_NaN()).exponent == fd128_exceptional_exponent);
    BOOST_TEST(floating_point_to_fd128(-std::numeric_limits<T>::infinity()).exponent == fd128_exceptional_exponent);
    BOOST_TEST(floating_point_to_fd128(-std::numeric_limits<T>::quiet_NaN()).exponent == fd128_exceptional_exponent);
}

int main()
{
    test_compute_float32();
    test_compute_float64();
    test_generic_binary_to_decimal<float>();
    test_generic_binary_to_decimal<double>();
    test_generic_binary_to_decimal<long double>();

    return boost::report_errors();
}
