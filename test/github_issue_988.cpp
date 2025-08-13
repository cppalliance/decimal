// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/cppalliance/decimal/issues/988

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

constexpr std::uint64_t value {0xb0005af3107a3ff0};

void test()
{
    using namespace boost::decimal;

    constexpr decimal64_t decimal_value {boost::decimal::from_bits(value)};

    // Regular to chars
    char buffer[256];
    const auto r = to_chars(buffer, buffer + sizeof(buffer), decimal_value);
    BOOST_TEST(r);
    *r.ptr = '\0';
    std::cerr << buffer << '\n';
}

int main()
{
    test();

    return boost::report_errors();
}
