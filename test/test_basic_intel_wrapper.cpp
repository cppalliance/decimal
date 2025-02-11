// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#define BOOST_DECIMAL_DEBUG_ACCESSORS

#include <boost/decimal/intel_decimal32.hpp>

#ifdef BOOST_DECIMAL_HAS_INTEL_WRAPPER

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/dpd_conversion.hpp>
#include <boost/core/lightweight_test.hpp>
#include <bitset>
#include <iostream>
#include <cstring>
#include <random>

void compare_bits(std::int32_t coeff, int)
{
    const boost::decimal::decimal32 dec32_val {coeff};
    const boost::decimal::intel_decimal32 intel_val {coeff};

    if (!BOOST_TEST_EQ(dec32_val.full_significand(), intel_val.full_significand()) ||
        !BOOST_TEST_EQ(dec32_val.biased_exponent(), intel_val.biased_exponent()) ||
        !BOOST_TEST_EQ(dec32_val.unbiased_exponent(), intel_val.unbiased_exponent()) ||
        !BOOST_TEST_EQ(dec32_val.isneg(), intel_val.isneg()))
    {
        // LCOV_EXCL_START
        std::uint32_t boost_bits;
        std::memcpy(&boost_bits, &dec32_val, sizeof(std::uint32_t));

        std::uint32_t gcc_bits;
        std::memcpy(&gcc_bits, &intel_val, sizeof(std::uint32_t));

        std::cerr << "Coeff: " << coeff << '\n'
                  << "Boost: " << std::bitset<32>(boost_bits) << "\n"
                  << "Intel: " << std::bitset<32>(gcc_bits) << "\n" << std::endl;
        // LCOV_EXCL_STOP
    }

    // Separate test of the to_components
    const auto dec_struct = dec32_val.to_components();
    const auto gcc_struct = intel_val.to_components();

    BOOST_TEST_EQ(gcc_struct.sign, intel_val.isneg());
    BOOST_TEST_EQ(gcc_struct.sig, intel_val.full_significand());
    BOOST_TEST_EQ(gcc_struct.exp, intel_val.biased_exponent());

    BOOST_TEST_EQ(dec_struct.sign, gcc_struct.sign);
    BOOST_TEST_EQ(dec_struct.sig, gcc_struct.sig);
    BOOST_TEST_EQ(dec_struct.exp, gcc_struct.exp);
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

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
