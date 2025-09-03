// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <cassert>

int main()
{
    BOOST_DECIMAL_ATTRIBUTE_UNUSED auto default_rounding_mode = boost::decimal::fegetround(); // Default is fe_dec_to_nearest

    BOOST_DECIMAL_ATTRIBUTE_UNUSED auto new_rounding_mode = boost::decimal::fesetround(boost::decimal::rounding_mode::fe_dec_to_nearest_from_zero);

    assert(default_rounding_mode != new_rounding_mode);

    return 0;
}

