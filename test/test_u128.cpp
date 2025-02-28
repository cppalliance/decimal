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
#include <cmath>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wundef"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wsign-conversion"
#  pragma clang diagnostic ignored "-Wfloat-equal"

#  if (__clang_major__ >= 10 && !defined(__APPLE__)) || __clang_major__ >= 13
#    pragma clang diagnostic ignored "-Wdeprecated-copy"
#  endif

#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/random/uniform_int_distribution.hpp>

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

// Used defined seed for repeatability
static std::mt19937_64 rng(42);

constexpr std::size_t N = 1024;

template <typename IntType>
void test_arithmetic_constructor()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        unsigned __int128 emulated_bits;
        std::memcpy(&emulated_bits, &emulated_value, sizeof(unsigned __int128));

        BOOST_TEST(emulated_bits == builtin_value);
    }
}

template <typename IntType>
void test_assignment_operators()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value;
        builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {};
        emulated_value = value;

        unsigned __int128 emulated_bits;
        std::memcpy(&emulated_bits, &emulated_value, sizeof(unsigned __int128));

        BOOST_TEST(emulated_bits == builtin_value);
    }
}

template <typename IntType>
void test_integer_conversion_operators()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value;
        builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {};
        emulated_value = value;

        const auto builtin_value_return = static_cast<IntType>(builtin_value);
        const auto emulated_value_return = static_cast<IntType>(emulated_value);

        BOOST_TEST_EQ(builtin_value_return, emulated_value_return);

        // Hits the implicit bool conversion
        if (builtin_value)
        {
            BOOST_TEST(emulated_value);
        }
    }
}

template <typename FloatType>
void test_float_conversion_operators()
{
    boost::random::uniform_int_distribution<std::uint64_t> dist(std::numeric_limits<std::uint64_t>::min(),
                                                                std::numeric_limits<std::uint64_t>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const auto value {dist(rng)};
        unsigned __int128 builtin_value;
        builtin_value = value;
        boost::decimal::detail::u128 emulated_value {};
        emulated_value = value;

        const auto builtin_value_return = static_cast<FloatType>(builtin_value);
        const auto emulated_value_return = static_cast<FloatType>(emulated_value);

        BOOST_TEST(std::abs(builtin_value_return - emulated_value_return) < std::numeric_limits<FloatType>::epsilon());
    }
}

template <typename IntType = std::uint64_t>
void test_unary_plus()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};
        emulated_value = +emulated_value;

        unsigned __int128 emulated_bits;
        std::memcpy(&emulated_bits, &emulated_value, sizeof(unsigned __int128));

        BOOST_TEST(emulated_bits == builtin_value);
    }
}

template <typename IntType = std::uint64_t>
void test_unary_minus()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        builtin_value = -builtin_value;
        boost::decimal::detail::u128 emulated_value {value};
        emulated_value = -emulated_value;

        unsigned __int128 emulated_bits;
        std::memcpy(&emulated_bits, &emulated_value, sizeof(unsigned __int128));

        BOOST_TEST(emulated_bits == builtin_value);
    }
}

template <typename IntType>
void test_operator_equality()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    // Always equal
    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST(((value == emulated_value) == (emulated_value == value)) ==
                   ((value == builtin_value) == (builtin_value == value)));
    }

    // Potentially equal
    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST(((value2 == emulated_value) == (emulated_value == value2)) ==
                   ((value2 == builtin_value) == (builtin_value == value2)));
    }

    boost::decimal::detail::u128 bool_val {dist(rng)};
    BOOST_TEST((true == bool_val) == (bool_val == true));
}

int main()
{
    test_arithmetic_constructor<std::int8_t>();
    test_arithmetic_constructor<std::int16_t>();
    test_arithmetic_constructor<std::int32_t>();
    test_arithmetic_constructor<std::int64_t>();
    test_arithmetic_constructor<__int128>();

    test_arithmetic_constructor<std::uint8_t>();
    test_arithmetic_constructor<std::uint16_t>();
    test_arithmetic_constructor<std::uint32_t>();
    test_arithmetic_constructor<std::uint64_t>();
    test_arithmetic_constructor<unsigned __int128>();

    test_assignment_operators<std::int8_t>();
    test_assignment_operators<std::int16_t>();
    test_assignment_operators<std::int32_t>();
    test_assignment_operators<std::int64_t>();
    test_arithmetic_constructor<__int128>();

    test_assignment_operators<std::uint8_t>();
    test_assignment_operators<std::uint16_t>();
    test_assignment_operators<std::uint32_t>();
    test_assignment_operators<std::uint64_t>();
    test_arithmetic_constructor<unsigned __int128>();

    test_integer_conversion_operators<std::int8_t>();
    test_integer_conversion_operators<std::int16_t>();
    test_integer_conversion_operators<std::int32_t>();
    test_integer_conversion_operators<std::int64_t>();
    test_arithmetic_constructor<__int128>();

    test_integer_conversion_operators<std::uint8_t>();
    test_integer_conversion_operators<std::uint16_t>();
    test_integer_conversion_operators<std::uint32_t>();
    test_integer_conversion_operators<std::uint64_t>();
    test_arithmetic_constructor<unsigned __int128>();

    test_float_conversion_operators<float>();
    test_float_conversion_operators<double>();
    test_float_conversion_operators<long double>();

    #ifdef BOOST_DECIMAL_HAS_FLOAT128
    test_float_conversion_operators<__float128>();
    #endif

    test_unary_plus();
    test_unary_minus();

    test_operator_equality<std::int8_t>();
    test_operator_equality<std::int16_t>();
    test_operator_equality<std::int32_t>();
    test_operator_equality<std::int64_t>();

    test_operator_equality<std::uint8_t>();
    test_operator_equality<std::uint16_t>();
    test_operator_equality<std::uint32_t>();
    test_operator_equality<std::uint64_t>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
