// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/cppalliance/decimal/issues/893

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <array>

void test_64()
{
    const std::array<std::uint64_t, 6> comp_values = {
        UINT64_C(12345678901234567),
        UINT64_C(123456789012345678),
        UINT64_C(1234567890123456789),
        UINT64_C(12345678901234567890),
    };

    int res {17};

    for (const auto& value : comp_values)
    {
        BOOST_TEST_EQ(boost::decimal::detail::num_digits(value), res);
        BOOST_TEST_EQ(boost::decimal::detail::d64_constructor_num_digits(value), res++);
    }
}

void test_128()
{
    const std::array<boost::int128::uint128_t, 6> comp_values = {
        BOOST_INT128_UINT128_C(1234567890123456789012345678901234),
        BOOST_INT128_UINT128_C(12345678901234567890123456789012345),
        BOOST_INT128_UINT128_C(123456789012345678901234567890123456),
        BOOST_INT128_UINT128_C(1234567890123456789012345678901234567),
        BOOST_INT128_UINT128_C(12345678901234567890123456789012345678),
        BOOST_INT128_UINT128_C(123456789012345678901234567890123456789),
    };

    int res {34};
    for (const auto& value : comp_values)
    {
        BOOST_TEST_EQ(boost::decimal::detail::num_digits(value), res);
        BOOST_TEST_EQ(boost::decimal::detail::d128_constructor_num_digits(value), res++);
    }

    #ifdef BOOST_DECIMAL_HAS_INT128

    res = 34;
    for (const auto& value : comp_values)
    {
        BOOST_TEST_EQ(boost::decimal::detail::num_digits(static_cast<boost::decimal::detail::builtin_uint128_t>(value)), res);
        BOOST_TEST_EQ(boost::decimal::detail::d128_constructor_num_digits(static_cast<boost::decimal::detail::builtin_uint128_t>(value)), res++);
    }

    #endif
}

int main()
{
    test_64();
    test_128();

    return boost::report_errors();
}
