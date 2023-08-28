// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <sstream>

using namespace boost::decimal;

void test_istream()
{

}

void test_ostream()
{
    decimal32 val(1234567, 0);
    std::stringstream out;
    out << val;
    BOOST_TEST_CSTR_EQ(out.str().c_str(), "1.234567e+06");

    decimal32 zero {0, 0};
    std::stringstream zero_out;
    zero_out << zero;
    BOOST_TEST_CSTR_EQ(zero_out.str().c_str(), "0.0e+00");
}

int main()
{
    test_istream();
    test_ostream();

    return boost::report_errors();
}
