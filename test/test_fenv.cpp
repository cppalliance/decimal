// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

int main()
{
    BOOST_TEST(boost::decimal::fegetround() == boost::decimal::rounding_mode::fe_dec_to_nearest_from_zero);
    BOOST_TEST(boost::decimal::fesetround(boost::decimal::rounding_mode::fe_dec_to_nearest) == boost::decimal::rounding_mode::fe_dec_to_nearest);
    BOOST_TEST(boost::decimal::fegetround() == boost::decimal::rounding_mode::fe_dec_to_nearest);
    BOOST_TEST(boost::decimal::fesetround(boost::decimal::rounding_mode::fe_dec_downward) == boost::decimal::rounding_mode::fe_dec_downward);
    BOOST_TEST(boost::decimal::fegetround() == boost::decimal::rounding_mode::fe_dec_downward);
    BOOST_TEST(boost::decimal::fesetround(boost::decimal::rounding_mode::fe_dec_upward) == boost::decimal::rounding_mode::fe_dec_upward);
    BOOST_TEST(boost::decimal::fegetround() == boost::decimal::rounding_mode::fe_dec_upward);

    return boost::report_errors();
}
