// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/detail/config.hpp>

#ifdef BOOST_DECIMAL_HAS_INT128

#include <boost/decimal/detail/u128.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>
#include <cstdint>
#include <random>
#include <limits>

// Used defined seed for repeatability
static std::mt19937_64 rng(42);

constexpr std::size_t N = 1024;

template <typename IntType>
void test_arithmetic_constructor()
{
    std::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value = value;
        boost::decimal::detail::u128 emulated_value {value};

        unsigned __int128 emulated_bits;
        std::memcpy(&emulated_bits, &emulated_value, sizeof(unsigned __int128));

        BOOST_TEST(emulated_bits == builtin_value);
    }
}

int main()
{
    test_arithmetic_constructor<std::int8_t>();
    test_arithmetic_constructor<std::int16_t>();
    test_arithmetic_constructor<std::int32_t>();
    test_arithmetic_constructor<std::int64_t>();

    test_arithmetic_constructor<std::uint8_t>();
    test_arithmetic_constructor<std::uint16_t>();
    test_arithmetic_constructor<std::uint32_t>();
    test_arithmetic_constructor<std::uint64_t>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
