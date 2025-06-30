// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

template <typename Dec>
void to_chars_helper()
{
    const Dec default_value;
    char buffer[64];

    const auto r = to_chars(buffer, buffer + sizeof(buffer), default_value);

    if (BOOST_TEST(r))
    {
        *r.ptr = '\0';
        BOOST_TEST_CSTR_EQ(buffer, "0.0e+00");
    }
}

int main()
{
    to_chars_helper<decimal32>();
    to_chars_helper<decimal64>();
    to_chars_helper<decimal128>();

    to_chars_helper<decimal32_fast>();
    to_chars_helper<decimal64_fast>();
    to_chars_helper<decimal128_fast>();

    return boost::report_errors();
}
