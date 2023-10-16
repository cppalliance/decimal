// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include <climits>

void test_compute_float32()
{
    using boost::decimal::detail::fast_float::compute_float32;

    std::mt19937_64 gen(42);
    std::uniform_int_distribution<std::int64_t> dist (1, 1);

    bool success {};

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

    std::mt19937_64 gen(42);
    std::uniform_int_distribution<std::int64_t> dist (1, 1);

    bool success {};

    // Trivial verifcation
    BOOST_TEST_EQ(compute_float64(1, dist(gen), false, success), 1e1);
    BOOST_TEST_EQ(compute_float64(0, dist(gen), true, success), -1e0);
    BOOST_TEST_EQ(compute_float64(308, dist(gen), false, success), 1e308);

    // out of range
    BOOST_TEST_EQ(compute_float64(310, 5 * dist(gen), false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(310, 5 * dist(gen), true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(1000, 5 * dist(gen), false, success), HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(1000, 5 * dist(gen), true, success), -HUGE_VALF);
    BOOST_TEST_EQ(compute_float64(-325, 5 * dist(gen), false, success), 0);
    BOOST_TEST_EQ(compute_float64(dist(gen) * 50, 0, false, success), 0);
    BOOST_TEST_EQ(compute_float64(dist(gen) * 50, 0, true, success), 0);
    BOOST_TEST_EQ(compute_float64(300, UINT64_MAX, false, success), 0 * dist(gen));

    // Composite
    BOOST_TEST_EQ(compute_float64(10 * dist(gen), 123456789, false, success), 123456789e10);
    BOOST_TEST_EQ(compute_float64(100 * dist(gen), UINT64_C(4444444444444444444), false, success), 4444444444444444444e100);
    BOOST_TEST_EQ(compute_float64(100 * dist(gen), std::numeric_limits<std::uint64_t>::max(), false, success), 18446744073709551615e100);
    BOOST_TEST_EQ(compute_float64(100 * dist(gen), UINT64_C(10000000000000000000), false, success), 10000000000000000000e100);
}

template <typename T>
void test_generic_binary_to_decimal()
{
    std::mt19937_64 gen(42);
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

    // https://github.com/cppalliance/decimal/issues/242
    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<T, double>::value)
    {
        // Consider the following integral patterns potentially stored
        // in a given decimal64.

        // 0x6A3A25E507BB83D9
        // 0x6A3800D0288A63E9
        // 0x6A380BF1150D7F35
        // 0x6A3B71802C99CB39

        std::array<std::uint64_t, 4> test_values {0x6A3A25E507BB83D9,
                                                  0x6A3800D0288A63E9,
                                                  0x6A380BF1150D7F35,
                                                  0x6A3B71802C99CB39};

        for (const auto bit_values : test_values)
        {
            const std::uint64_t ui = bit_values;

            // And we convert it via direct-memory access to a decimal64.

            // In this particular example we assume that this number could
            // be the result of some calculation, initialization or any other
            // valid operations.

            const boost::decimal::decimal64 dec = *reinterpret_cast<const boost::decimal::decimal64*>(&ui);

            // Now let's convert this decimal64 value to built-in double.
            // ISSUE: This cast seems to result in 0.
            const auto dbl = static_cast<double>(dec);
            const boost::decimal::decimal64 return_dec(dbl);

            if (!BOOST_TEST(dec == return_dec))
            {
                std::cerr << std::scientific << std::setprecision(std::numeric_limits<double>::digits10)
                          << "       Dec: " << dec
                          << "\n       Dbl: " << dbl
                          << "\nReturn Dec: " << return_dec << std::endl;
            }
        }
    }
}

void test_parser()
{
    using namespace boost::decimal::detail;

    std::mt19937_64 gen(42);
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

    const char* big_exp = "12345.6789e+1000000";
    res = parser(big_exp, big_exp + std::strlen(big_exp), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc::result_out_of_range);

    const char* zeros = "0.00000000";
    res = parser(zeros, zeros + std::strlen(zeros), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* only_exp = "e+03";
    res = parser(only_exp, only_exp + std::strlen(only_exp), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc::invalid_argument);

    const char* fives = "5.555555555555555555555555555555e+05";
    res = parser(fives, fives + std::strlen(fives), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* sixes = "6.6666666666666666666666666666666e+06";
    res = parser(sixes, sixes + std::strlen(sixes), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* sevens = "7.777777777777777777777777777777e+07";
    res = parser(sevens, sevens + std::strlen(sevens), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* eights = "8.888888888888888888888888888888e+08";
    res = parser(eights, eights + std::strlen(eights), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());

    const char* nines = "9.99999999999999999999999999999999e+09";
    res = parser(nines, nines + std::strlen(nines), sign, sig, exp);
    BOOST_TEST(res.ec == std::errc());
}

void test_from_chars()
{
    using namespace boost::decimal::detail;

    std::mt19937_64 gen(42);
    std::uniform_int_distribution<std::uint64_t> dist (1, 2);

    const char* pos_str = "+12345";
    std::uint64_t sig {dist(gen)};

    auto res = from_chars_integer_impl<std::uint64_t, std::uint64_t>(pos_str, pos_str, sig, 10);
    BOOST_TEST(res.ec == std::errc::invalid_argument);
    res = from_chars_integer_impl<std::uint64_t, std::uint64_t>(pos_str, pos_str + std::strlen(pos_str), sig, 10);
    BOOST_TEST(res.ec == std::errc::invalid_argument);

    const char* neg_str = "-12345";
    res = from_chars_integer_impl<std::uint64_t, std::uint64_t>(neg_str, neg_str + std::strlen(neg_str), sig, 10);
    BOOST_TEST(res.ec == std::errc::invalid_argument);

    auto signed_sig {static_cast<std::int64_t>(dist(gen))};
    res = from_chars_integer_impl<std::int64_t, std::uint64_t>(neg_str, neg_str + 1, signed_sig, 10);
    BOOST_TEST(res.ec == std::errc::invalid_argument);

    res = from_chars_integer_impl<std::int64_t, std::uint64_t>(neg_str, neg_str + std::strlen(neg_str), signed_sig, 10);
    BOOST_TEST(res.ec == std::errc());

    const char* big_sig = "-123456789123456789123456789123456789";
    res = from_chars_integer_impl<std::int64_t, std::uint64_t>(big_sig, big_sig + std::strlen(big_sig), signed_sig, 10);
    BOOST_TEST(res.ec == std::errc::result_out_of_range);
}

int main()
{
    test_compute_float32();
    test_compute_float64();
    test_generic_binary_to_decimal<float>();
    test_generic_binary_to_decimal<double>();
    test_generic_binary_to_decimal<long double>();

    test_parser();

    test_from_chars();

    return boost::report_errors();
}
