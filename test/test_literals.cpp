// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <cfloat>
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

void test_decimal32_literals()
{
    BOOST_TEST_EQ(decimal32(0), 0_DF);
    BOOST_TEST_EQ(decimal32(3), 3_DF);
    BOOST_TEST_EQ(decimal32(3.1), 3.1_DF);
    BOOST_TEST_EQ(decimal32(3, 1), 3e1_DF);
    BOOST_TEST(isinf(5e100_DF));
    BOOST_TEST(isinf(5e300_DF));

    BOOST_TEST_EQ(decimal32(0), 0_df);
    BOOST_TEST_EQ(decimal32(3), 3_df);
    BOOST_TEST_EQ(decimal32(3.1), 3.1_df);
    BOOST_TEST_EQ(decimal32(3, 1), 3e1_df);
    BOOST_TEST(isinf(5e100_df));
    BOOST_TEST(isinf(5e300_df));
}

void test_decimal64_literals()
{
    BOOST_TEST_EQ(decimal64(0), 0_DD);
    BOOST_TEST_EQ(decimal64(3), 3_DD);
    BOOST_TEST_EQ(decimal64(3.1), 3.1_DD);
    BOOST_TEST_EQ(decimal64(3, 1), 3e1_DD);

    BOOST_TEST_EQ(decimal64(0), 0_dd);
    BOOST_TEST_EQ(decimal64(3), 3_dd);
    BOOST_TEST_EQ(decimal64(3.1), 3.1_dd);
    BOOST_TEST_EQ(decimal64(3, 1), 3e1_dd);

    // 64-bit long double warn of overflow
    #if !(LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024)
    BOOST_TEST(isinf(5e1000_dd));
    BOOST_TEST(isinf(5e3000_dd));
    BOOST_TEST(isinf(5e1000_DD));
    BOOST_TEST(isinf(5e3000_DD));
    #endif
}

int main()
{
    test_decimal32_literals();
    test_decimal64_literals();

    return boost::report_errors();
}
