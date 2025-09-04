// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/cppalliance/decimal/issues/1026

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>

using namespace boost::decimal;
using namespace boost::decimal::literals;

int main()
{
    // Round ties to even

    BOOST_TEST_EQ("1234567.49"_DF, "1234567"_DF);
    BOOST_TEST_EQ("1234567.50"_DF, "1234568"_DF);
    BOOST_TEST_EQ("1234567.51"_DF, "1234568"_DF);

    BOOST_TEST_EQ("2345678.49"_DF, "2345678"_DF);
    BOOST_TEST_EQ("2345678.50"_DF, "2345678"_DF);
    BOOST_TEST_EQ("2345678.51"_DF, "2345679"_DF);

    BOOST_TEST_EQ(("0"_DF + "8.4e-96"_DF), "8.4e-96"_DF);
    BOOST_TEST_EQ(("0"_DF + std::numeric_limits<decimal32_t>::denorm_min()), std::numeric_limits<decimal32_t>::denorm_min());
    BOOST_TEST_EQ((std::numeric_limits<decimal32_t>::denorm_min() + std::numeric_limits<decimal32_t>::denorm_min()), 2*std::numeric_limits<decimal32_t>::denorm_min());

    return boost::report_errors();
}

