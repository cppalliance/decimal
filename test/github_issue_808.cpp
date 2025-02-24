// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

template <typename Dec>
void tiny_div()
{
    constexpr Dec zero {0};

    const auto tiny = std::numeric_limits<Dec>::denorm_min();
    const auto tiny2 = tiny / Dec{10};
    BOOST_TEST(tiny2 == zero);
}

int main()
{
    tiny_div<decimal32>();
    tiny_div<decimal64>();
    tiny_div<decimal128>();

    tiny_div<decimal32_fast>();
    tiny_div<decimal64_fast>();
    tiny_div<decimal128_fast>();

    return boost::report_errors();
}
