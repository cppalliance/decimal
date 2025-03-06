// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/uint128.hpp>
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
#  pragma GCC diagnostic ignored "-Wsign-compare"
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/random/uniform_int_distribution.hpp>

// Used defined seed for repeatability
static std::mt19937_64 rng(42);

constexpr std::size_t N = 1024;

void test_traits()
{
    using namespace boost::decimal::detail::impl;

    static_assert(is_signed_integer_v<std::int8_t> && !is_unsigned_integer_v<std::int8_t>, "Wrong answer");
    static_assert(is_signed_integer_v<std::int16_t> && !is_unsigned_integer_v<std::int16_t>, "Wrong answer");
    static_assert(is_signed_integer_v<std::int32_t> && !is_unsigned_integer_v<std::int32_t>, "Wrong answer");
    static_assert(is_signed_integer_v<std::int64_t> && !is_unsigned_integer_v<std::int64_t>, "Wrong answer");

    static_assert(!is_signed_integer_v<std::uint8_t> && is_unsigned_integer_v<std::uint8_t>, "Wrong answer");
    static_assert(!is_signed_integer_v<std::uint16_t> && is_unsigned_integer_v<std::uint16_t>, "Wrong answer");
    static_assert(!is_signed_integer_v<std::uint32_t> && is_unsigned_integer_v<std::uint32_t>, "Wrong answer");
    static_assert(!is_signed_integer_v<std::uint64_t> && is_unsigned_integer_v<std::uint64_t>, "Wrong answer");

    static_assert(!is_signed_integer_v<float> && !is_unsigned_integer_v<float>, "Wrong answer");
    static_assert(!is_signed_integer_v<double> && !is_unsigned_integer_v<double>, "Wrong answer");
    static_assert(!is_signed_integer_v<long double> && !is_unsigned_integer_v<long double>, "Wrong answer");
}

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

template <typename IntType>
void test_operator_inequality()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    // Always equal
    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST(((value != emulated_value) == (emulated_value != value)) ==
                   ((value != builtin_value) == (builtin_value != value)));
    }

    // Potentially equal
    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST(((value2 != emulated_value) == (emulated_value != value2)) ==
                   ((value2 != builtin_value) == (builtin_value != value2)));
    }

    boost::decimal::detail::u128 bool_val {dist(rng)};
    BOOST_TEST((true != bool_val) == (bool_val != true));
}

template <typename IntType>
void test_operator_less()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        // Some platforms get this wrong where for example -99 < 340282366920938463463374607431768211408 evaluates to false
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4127)
        #endif

        BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<IntType>::value)
        {
            if (value == value2 && value < 0)
            {
                continue;
            }
        }

        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif

        BOOST_TEST(((value2 < emulated_value) == (value2 < builtin_value)) ==
                   ((emulated_value < value2) == (builtin_value < value2)));
    }
}

template <typename IntType>
void test_operator_le()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        // Some platforms get this wrong where for example -99 < 340282366920938463463374607431768211408 evaluates to false
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4127)
        #endif

        BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<IntType>::value)
        {
            if (value == value2 && value < 0)
            {
                continue;
            }
        }

        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif

        BOOST_TEST(((value2 <= emulated_value) == (value2 <= builtin_value)) ==
                   ((emulated_value <= value2) == (builtin_value <= value2)));
    }
}

template <typename IntType>
void test_operator_greater()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        // Some platforms get this wrong where for example -99 < 340282366920938463463374607431768211408 evaluates to false
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4127)
        #endif

        BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<IntType>::value)
        {
            if (value == value2 && value < 0)
            {
                continue;
            }
        }

        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif

        BOOST_TEST(((value2 > emulated_value) == (value2 > builtin_value)) ==
                   ((emulated_value > value2) == (builtin_value > value2)));
    }
}

template <typename IntType>
void test_operator_ge()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        // Some platforms get this wrong where for example -99 < 340282366920938463463374607431768211408 evaluates to false
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4127)
        #endif

        BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<IntType>::value)
        {
            if (value == value2 && value < 0)
            {
                continue;
            }
        }

        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif

        BOOST_TEST(((value2 >= emulated_value) == (value2 >= builtin_value)) ==
                   ((emulated_value >= value2) == (builtin_value >= value2)));
    }
}

template <typename IntType = unsigned __int128>
void test_operator_not()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST(~emulated_value == ~builtin_value);
    }
}

template <typename IntType>
void test_operator_or()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value | value2) == (builtin_value | value2));
        BOOST_TEST((value2 | emulated_value) == (value2 | builtin_value));
    }
}

template <typename IntType>
void test_operator_and()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value & value2) == (builtin_value & value2));
        BOOST_TEST((value2 & emulated_value) == (value2 & builtin_value));
    }
}

template <typename IntType>
void test_operator_xor()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value ^ value2) == (builtin_value ^ value2));
        BOOST_TEST((value2 ^ emulated_value) == (value2 ^ builtin_value));
    }
}

template <typename IntType>
void test_operator_left_shift()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    boost::random::uniform_int_distribution<unsigned> shift_dist(0, sizeof(IntType) * CHAR_BIT - 1);

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const unsigned shift_value {shift_dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value << shift_value) == (builtin_value << shift_value));

        emulated_value = shift_value;
        builtin_value = shift_value;
        BOOST_TEST((value << emulated_value) == (value << builtin_value));
    }
}

template <typename IntType>
void test_operator_right_shift()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    boost::random::uniform_int_distribution<unsigned> shift_dist(0, sizeof(IntType) * CHAR_BIT - 1);

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const unsigned shift_value {shift_dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value >> shift_value) == (builtin_value >> shift_value));

        emulated_value = shift_value;
        builtin_value = shift_value;
        BOOST_TEST((value >> emulated_value) == (value >> builtin_value));
    }
}

template <typename IntType>
void test_operator_add()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value + value2) == (builtin_value + value2));
        BOOST_TEST((value2 + emulated_value) == (value2 + builtin_value));
    }
}

template <typename IntType>
void test_operator_sub()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value + value2) == (builtin_value + value2));
        BOOST_TEST((value2 + emulated_value) == (value2 + builtin_value));
    }
}

template <typename IntType, std::enable_if_t<(sizeof(IntType) <= sizeof(std::uint64_t)), bool> = true>
void test_operator_mul()
{
    const auto root_max {static_cast<IntType>(std::sqrt(std::numeric_limits<IntType>::max()))};
    const auto root_min {std::is_unsigned<IntType>::value ? 0 : -root_max};

    boost::random::uniform_int_distribution<IntType> dist(root_min, root_max);

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value * value2) == (builtin_value * value2));
        BOOST_TEST((value2 * emulated_value) == (value2 * builtin_value));
    }
}

template <typename IntType, std::enable_if_t<(sizeof(IntType) > sizeof(std::uint64_t)), bool> = true>
void test_operator_mul()
{
    constexpr auto root_max {UINT64_MAX};
    const auto root_min {std::is_same<IntType, unsigned __int128>::value ? 0 : -root_max};

    boost::random::uniform_int_distribution<IntType> dist(root_min, root_max);

    for (std::size_t i {}; i < N; ++i)
    {
        const IntType value {dist(rng)};
        const IntType value2 {dist(rng)};
        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        BOOST_TEST((emulated_value * value2) == (builtin_value * value2));
        BOOST_TEST((value2 * emulated_value) == (value2 * builtin_value));
    }
}

int main()
{
    test_traits();

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
    test_operator_equality<__int128>();

    test_operator_equality<std::uint8_t>();
    test_operator_equality<std::uint16_t>();
    test_operator_equality<std::uint32_t>();
    test_operator_equality<std::uint64_t>();
    test_operator_equality<unsigned __int128>();

    test_operator_inequality<std::int8_t>();
    test_operator_inequality<std::int16_t>();
    test_operator_inequality<std::int32_t>();
    test_operator_inequality<std::int64_t>();
    test_operator_inequality<__int128>();

    test_operator_inequality<std::uint8_t>();
    test_operator_inequality<std::uint16_t>();
    test_operator_inequality<std::uint32_t>();
    test_operator_inequality<std::uint64_t>();
    test_operator_inequality<unsigned __int128>();

    test_operator_less<std::int8_t>();
    test_operator_less<std::int16_t>();
    test_operator_less<std::int32_t>();
    test_operator_less<std::int64_t>();
    test_operator_less<__int128>();

    test_operator_less<std::uint8_t>();
    test_operator_less<std::uint16_t>();
    test_operator_less<std::uint32_t>();
    test_operator_less<std::uint64_t>();
    test_operator_less<unsigned __int128>();

    test_operator_le<std::int8_t>();
    test_operator_le<std::int16_t>();
    test_operator_le<std::int32_t>();
    test_operator_le<std::int64_t>();
    test_operator_le<__int128>();

    test_operator_le<std::uint8_t>();
    test_operator_le<std::uint16_t>();
    test_operator_le<std::uint32_t>();
    test_operator_le<std::uint64_t>();
    test_operator_le<unsigned __int128>();

    test_operator_greater<std::int8_t>();
    test_operator_greater<std::int16_t>();
    test_operator_greater<std::int32_t>();
    test_operator_greater<std::int64_t>();
    test_operator_greater<__int128>();

    test_operator_greater<std::uint8_t>();
    test_operator_greater<std::uint16_t>();
    test_operator_greater<std::uint32_t>();
    test_operator_greater<std::uint64_t>();
    test_operator_greater<unsigned __int128>();

    test_operator_ge<std::int8_t>();
    test_operator_ge<std::int16_t>();
    test_operator_ge<std::int32_t>();
    test_operator_ge<std::int64_t>();
    test_operator_ge<__int128>();

    test_operator_ge<std::uint8_t>();
    test_operator_ge<std::uint16_t>();
    test_operator_ge<std::uint32_t>();
    test_operator_ge<std::uint64_t>();
    test_operator_ge<unsigned __int128>();

    test_operator_not();

    test_operator_or<std::int8_t>();
    test_operator_or<std::int16_t>();
    test_operator_or<std::int32_t>();
    test_operator_or<std::int64_t>();
    test_operator_or<__int128>();

    test_operator_or<std::uint8_t>();
    test_operator_or<std::uint16_t>();
    test_operator_or<std::uint32_t>();
    test_operator_or<std::uint64_t>();
    test_operator_or<unsigned __int128>();

    test_operator_and<std::int8_t>();
    test_operator_and<std::int16_t>();
    test_operator_and<std::int32_t>();
    test_operator_and<std::int64_t>();
    test_operator_and<__int128>();

    test_operator_and<std::uint8_t>();
    test_operator_and<std::uint16_t>();
    test_operator_and<std::uint32_t>();
    test_operator_and<std::uint64_t>();
    test_operator_and<unsigned __int128>();

    test_operator_xor<std::int8_t>();
    test_operator_xor<std::int16_t>();
    test_operator_xor<std::int32_t>();
    test_operator_xor<std::int64_t>();
    test_operator_xor<__int128>();

    test_operator_xor<std::uint8_t>();
    test_operator_xor<std::uint16_t>();
    test_operator_xor<std::uint32_t>();
    test_operator_xor<std::uint64_t>();
    test_operator_xor<unsigned __int128>();

    test_operator_left_shift<std::int8_t>();
    test_operator_left_shift<std::int16_t>();
    test_operator_left_shift<std::int32_t>();
    test_operator_left_shift<std::int64_t>();
    test_operator_left_shift<__int128>();

    test_operator_left_shift<std::uint8_t>();
    test_operator_left_shift<std::uint16_t>();
    test_operator_left_shift<std::uint32_t>();
    test_operator_left_shift<std::uint64_t>();
    test_operator_left_shift<unsigned __int128>();

    test_operator_right_shift<std::int8_t>();
    test_operator_right_shift<std::int16_t>();
    test_operator_right_shift<std::int32_t>();
    test_operator_right_shift<std::int64_t>();
    test_operator_right_shift<__int128>();

    test_operator_right_shift<std::uint8_t>();
    test_operator_right_shift<std::uint16_t>();
    test_operator_right_shift<std::uint32_t>();
    test_operator_right_shift<std::uint64_t>();
    test_operator_right_shift<unsigned __int128>();

    test_operator_add<std::int8_t>();
    test_operator_add<std::int16_t>();
    test_operator_add<std::int32_t>();
    test_operator_add<std::int64_t>();
    test_operator_add<__int128>();

    test_operator_add<std::uint8_t>();
    test_operator_add<std::uint16_t>();
    test_operator_add<std::uint32_t>();
    test_operator_add<std::uint64_t>();
    test_operator_add<unsigned __int128>();

    test_operator_sub<std::int8_t>();
    test_operator_sub<std::int16_t>();
    test_operator_sub<std::int32_t>();
    test_operator_sub<std::int64_t>();
    test_operator_sub<__int128>();

    test_operator_sub<std::uint8_t>();
    test_operator_sub<std::uint16_t>();
    test_operator_sub<std::uint32_t>();
    test_operator_sub<std::uint64_t>();
    test_operator_sub<unsigned __int128>();

    test_operator_mul<std::int8_t>();
    test_operator_mul<std::int16_t>();
    test_operator_mul<std::int32_t>();
    test_operator_mul<std::int64_t>();
    test_operator_mul<__int128>();

    test_operator_mul<std::uint8_t>();
    test_operator_mul<std::uint16_t>();
    test_operator_mul<std::uint32_t>();
    test_operator_mul<std::uint64_t>();
    test_operator_mul<unsigned __int128>();

    return boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
