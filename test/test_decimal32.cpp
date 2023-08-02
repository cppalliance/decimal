// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cassert>

int main()
{
    boost::decimal::decimal32 small(1, -50);
    BOOST_TEST(small == small);

    std::cerr << small << std::endl;

    boost::decimal::decimal32 sig(123456, -50);
    BOOST_TEST(sig != small);

    std::cerr << sig << std::endl;

    return boost::report_errors();
}
