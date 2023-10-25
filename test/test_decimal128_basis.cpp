// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <bitset>
#include <limits>
#include <random>
#include <cmath>
#include <cerrno>

using namespace boost::decimal;

#if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE && defined(BOOST_DECIMAL_HAS_INT128)
void test_binary_constructor()
{
    decimal128 one(0b1, -6175);
    // 0 for sign
    // 00000 for combination field
    // 00000000 for exp
    // 1 for significand
    BOOST_TEST_EQ(bit_string(one), "4000000000001");

    decimal128 neg_one(-0b1, -6175);
    // 1 for sign
    // 00000 for combination field
    // 00000000 for exp
    // 1 for significand
    BOOST_TEST_EQ(bit_string(neg_one), "80004000000000001");

    decimal128 two(0b10, -6175);
    // 0 for sign
    // 00000 for combination field
    // 00000000 for exp
    // 2 for significand
    BOOST_TEST_EQ(bit_string(two), "4000000000002");

    decimal128 three(0b11, -6175);
    // 0 for sign
    // 00000 for combination field
    // 00000000 for exp
    // 3 for significand
    BOOST_TEST_EQ(bit_string(three), "4000000000003");

    decimal128 big(UINT64_MAX, -6175);
    // 0 for sign
    // 00000 for combination field
    // 00000000 for exp
    // FFFFFFFFFFFFFFFF for significand
    BOOST_TEST_EQ(bit_string(big), "400000000000ffffffffffffffff");

    decimal128 onee1(0b1, -6174);
    // 0 for sign
    // 00000 for combination field
    // 00000001 for exp
    // 1 for significand
    BOOST_TEST_EQ(bit_string(onee1), "8000000000001");
}

#else

void test_binary_constructor()
{
    //nothing
}

#endif

int main()
{
    test_binary_constructor();

    return boost::report_errors();
}
