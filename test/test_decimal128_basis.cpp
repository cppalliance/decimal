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
    // 1 for sign
    // 00000 for combination field
    // 00000000 for exp
    // 1 for significand
    BOOST_TEST_EQ(std::bitset<128>(to_bits(one)),
                  std::bitset<128>("0000000000000000000000000000000000000000000000000000000000000000"
                                   "0000000000000000000000000000000000000000000000000000000000000001"));

    decimal128 neg_one(-0b1, -6175);
    // 1 for sign
    // 00000 for combination field
    // 00000000 for exp
    // 1 for significand
    BOOST_TEST_EQ(std::bitset<128>(to_bits(neg_one)),
                  std::bitset<128>("1000000000000000000000000000000000000000000000000000000000000000"
                                   "0000000000000000000000000000000000000000000000000000000000000001"));;
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
