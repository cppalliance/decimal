// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/cppalliance/decimal/issues/988

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

constexpr std::uint64_t value {0xb0005af3107a3ff0};
constexpr decimal64_t decimal_value {from_bits(value)};


void test()
{
    // Regular to chars
    char buffer[256];
    const auto r = to_chars(buffer, buffer + sizeof(buffer), decimal_value);
    BOOST_TEST(r);
    *r.ptr = '\0';
    BOOST_TEST_CSTR_EQ(buffer, "-9.9999999999984e-01");
}

void fixed_precision_test()
{
    char buffer[256];
    const auto r = to_chars(buffer, buffer + sizeof(buffer), decimal_value, chars_format::general, 6);
    BOOST_TEST(r);
    *r.ptr = '\0';
    BOOST_TEST_CSTR_EQ(buffer, "-1");
}

int main()
{
    test();
    fixed_precision_test();

    return boost::report_errors();
}
