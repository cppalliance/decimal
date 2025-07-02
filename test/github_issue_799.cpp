// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>

#ifdef BOOST_DECIMAL_HAS_INT128

template<typename Dec>
void mixed_compare()
{
    const Dec a{2, 1};
    const auto b = __uint128_t{std::numeric_limits<__uint128_t>::max() - std::numeric_limits<uint64_t>::max() + 20};
    BOOST_TEST(a != b);
}

int main()
{
    using namespace boost::decimal;

    mixed_compare<decimal32>();
    mixed_compare<decimal32_fast>();
    mixed_compare<decimal64>();
    mixed_compare<decimal64_fast>();
    mixed_compare<decimal128>();
    mixed_compare<decimal128_fast>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
