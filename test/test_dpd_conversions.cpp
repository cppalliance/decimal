// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>

using namespace boost::decimal;

template <typename T>
T roundtrip(T val)
{
    const auto bits {to_dpd(val)};
    return from_dpd<T>(bits);
}

template <typename T>
void test()
{
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<std::int64_t> dist(std::numeric_limits<std::int64_t>::min(),
                                                     std::numeric_limits<std::int64_t>::max());

    for (std::size_t i {}; i < 1024; ++i)
    {
        const T val {dist(rng)};
        const T return_val {roundtrip(val)};
        BOOST_TEST_EQ(val, return_val);
    }

    // Non-finite values
    BOOST_TEST(isinf(roundtrip(std::numeric_limits<T>::infinity())));
    BOOST_TEST(isinf(roundtrip(-std::numeric_limits<T>::infinity())));
    BOOST_TEST(isnan(roundtrip(std::numeric_limits<T>::quiet_NaN())));
    BOOST_TEST(isnan(roundtrip(-std::numeric_limits<T>::quiet_NaN())));
    BOOST_TEST(isnan(roundtrip(std::numeric_limits<T>::signaling_NaN())));
    BOOST_TEST(isnan(roundtrip(-std::numeric_limits<T>::signaling_NaN())));
}

int main()
{
    test<decimal32>();
    test<decimal32_fast>();

    return boost::report_errors();
}
