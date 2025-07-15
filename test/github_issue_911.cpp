// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/cppalliance/decimal/issues/911

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>

int main()
{
    // -7.50
    constexpr boost::decimal::decimal128 test_val {-750, -2};
    boost::int128::uint128_t bits {};
    std::memcpy(&bits, &test_val, sizeof(bits));

    BOOST_TEST_EQ(bits.low, UINT64_C(750));
    BOOST_TEST_EQ(bits.high, UINT64_C(12699025049277956096));

    return boost::report_errors();
}
