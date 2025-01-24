// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#define BOOST_DECIMAL_DEBUG_ACCESSORS

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/gcc_decimal32.hpp>
#include <boost/decimal/dpd_conversion.hpp>
#include <boost/core/lightweight_test.hpp>
#include <bitset>
#include <iostream>
#include <cstring>
#include <random>

void compare_bits(long long coeff, int exp)
{
    const boost::decimal::decimal32 dec32_val {coeff, exp};
    const boost::decimal::gcc_decimal32 gcc_val {coeff, exp};

    if (!BOOST_TEST_EQ(dec32_val.full_significand(), gcc_val.full_significand()) ||
        !BOOST_TEST_EQ(dec32_val.biased_exponent(), gcc_val.biased_exponent()) ||
        !BOOST_TEST_EQ(dec32_val.unbiased_exponent(), gcc_val.unbiased_exponent()) ||
        !BOOST_TEST_EQ(dec32_val.isneg(), gcc_val.isneg()))
    {
        std::uint32_t boost_bits;
        std::memcpy(&boost_bits, &dec32_val, sizeof(std::uint32_t));

        std::uint32_t gcc_bits;
        std::memcpy(&gcc_bits, &gcc_val, sizeof(std::uint32_t));

        std::cerr << "Coeff: " << coeff << '\n'
                  << "  Exp: " << exp << '\n'
                  << "Boost: " << std::bitset<32>(boost_bits) << "\n"
                  << "  GCC: " << std::bitset<32>(gcc_bits) << "\n" << std::endl;
    }
}

int main()
{
    // 2^0
    compare_bits(1, 0);

    // 2^1
    compare_bits(2, 0);

    // 2^21
    compare_bits(2097152, 0);

    // 2^22
    compare_bits(4194304, 0);

    // 2^23
    compare_bits(8388608, 0);

    // 2^24
    // Exceeds digits10 so needs to be rounded
    compare_bits(16777216, 0);

    // Random significands with 0 exponent
    std::mt19937_64 rng{42};
    std::uniform_int_distribution<long long> sig_dist(-10000000, 1000000);
    for (int i {}; i < 1024; ++i)
    {
        compare_bits(sig_dist(rng), 0);
    }

    // Random powers of 2
    std::uniform_int_distribution<int> exp_dist(-88, 89);
    for (int i {}; i < 1024; ++i)
    {
        compare_bits(2, exp_dist(rng));
    }

    // Put it all together now
    for (int i {}; i < 1024; ++i)
    {
        compare_bits(sig_dist(rng), exp_dist(rng));
    }

    return boost::report_errors();
}
