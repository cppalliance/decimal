// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/uint128.hpp>
#include <boost/decimal/detail/config.hpp>

#include <boost/decimal/detail/u128.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstring>
#include <cstdint>
#include <random>
#include <limits>
#include <cmath>
#include <sstream>
#include <iostream>
#include <iomanip>

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

#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4389)
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

void test_numeric_limits()
{
    using namespace boost::decimal::detail;

    BOOST_TEST(std::numeric_limits<u128>::is_specialized);
    BOOST_TEST(!std::numeric_limits<u128>::is_signed);
    BOOST_TEST(std::numeric_limits<u128>::is_integer);
    BOOST_TEST(std::numeric_limits<u128>::is_exact);
    BOOST_TEST(!std::numeric_limits<u128>::has_infinity);
    BOOST_TEST(!std::numeric_limits<u128>::has_quiet_NaN);
    BOOST_TEST(!std::numeric_limits<u128>::has_signaling_NaN);

    BOOST_TEST(std::numeric_limits<u128>::round_style == std::round_toward_zero);
    BOOST_TEST(!std::numeric_limits<u128>::is_iec559);
    BOOST_TEST(std::numeric_limits<u128>::is_bounded);
    BOOST_TEST(std::numeric_limits<u128>::is_modulo);
    BOOST_TEST_EQ(std::numeric_limits<u128>::digits, sizeof(u128) * 8U);
    BOOST_TEST_EQ(std::numeric_limits<u128>::digits10, static_cast<int>(std::numeric_limits<u128>::digits * std::log10(2)));
    BOOST_TEST_EQ(std::numeric_limits<u128>::max_digits10, std::numeric_limits<std::uint64_t>::max_digits10);
    BOOST_TEST_EQ(std::numeric_limits<u128>::radix, std::numeric_limits<std::uint64_t>::radix);
    BOOST_TEST_EQ(std::numeric_limits<u128>::min_exponent, std::numeric_limits<std::uint64_t>::min_exponent);
    BOOST_TEST_EQ(std::numeric_limits<u128>::min_exponent10, std::numeric_limits<std::uint64_t>::min_exponent10);
    BOOST_TEST_EQ(std::numeric_limits<u128>::max_exponent, std::numeric_limits<std::uint64_t>::max_exponent);
    BOOST_TEST_EQ(std::numeric_limits<u128>::max_exponent10, std::numeric_limits<std::uint64_t>::max_exponent10);
    BOOST_TEST_EQ(std::numeric_limits<u128>::traps, std::numeric_limits<std::uint64_t>::traps);
    BOOST_TEST_EQ(std::numeric_limits<u128>::tinyness_before, std::numeric_limits<std::uint64_t>::tinyness_before);

    BOOST_TEST(std::numeric_limits<u128>::min() == std::numeric_limits<std::uint64_t>::min());
    BOOST_TEST(std::numeric_limits<u128>::lowest() == std::numeric_limits<std::uint64_t>::lowest());

    #ifndef BOOST_DECIMAL_HAS_INT128

    constexpr u128 two128 {std::numeric_limits<std::uint64_t>::max(), std::numeric_limits<std::uint64_t>::max()};
    BOOST_TEST(std::numeric_limits<u128>::max() == two128);

    #else

    unsigned __int128 max_value {std::numeric_limits<std::uint64_t>::max()};
    max_value <<= 64U;
    max_value |= std::numeric_limits<std::uint64_t>::max();
    BOOST_TEST(std::numeric_limits<u128>::max() == max_value);

    #endif

    BOOST_TEST(std::numeric_limits<u128>::epsilon() == std::numeric_limits<std::uint64_t>::epsilon());
    BOOST_TEST(std::numeric_limits<u128>::round_error() == std::numeric_limits<std::uint64_t>::round_error());
    BOOST_TEST(std::numeric_limits<u128>::infinity() == std::numeric_limits<std::uint64_t>::infinity());
    BOOST_TEST(std::numeric_limits<u128>::quiet_NaN() == std::numeric_limits<std::uint64_t>::quiet_NaN());
    BOOST_TEST(std::numeric_limits<u128>::signaling_NaN() == std::numeric_limits<std::uint64_t>::signaling_NaN());
    BOOST_TEST(std::numeric_limits<u128>::denorm_min() == std::numeric_limits<std::uint64_t>::denorm_min());
}

#ifdef BOOST_DECIMAL_HAS_INT128

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

    // Float128 won't have numerics limit defined all the time,
    // Plus this affords some additional tolerance
    constexpr FloatType error_tol {std::is_same<FloatType, float>::value ?
        static_cast<FloatType>(std::numeric_limits<float>::epsilon()) :
        static_cast<FloatType>(std::numeric_limits<double>::epsilon())};

    for (std::size_t i {}; i < N; ++i)
    {
        const auto value {dist(rng)};
        unsigned __int128 builtin_value;
        builtin_value = value;
        boost::decimal::detail::u128 emulated_value {};
        emulated_value = value;

        const auto builtin_value_return = static_cast<FloatType>(builtin_value);
        const auto emulated_value_return = static_cast<FloatType>(emulated_value);
        FloatType distance = builtin_value_return - emulated_value_return;

        // We don't want to pull in quad math for a simple abs calculation...
        distance = distance < 0 ? -distance : distance;

        BOOST_TEST(distance < error_tol);
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

        auto check_1_value {emulated_value};
        check_1_value |= value2;

        BOOST_TEST(check_1_value == (builtin_value | value2));
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

        auto check_1_value {emulated_value};
        check_1_value &= value2;

        BOOST_TEST(check_1_value == (builtin_value & value2));
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

        auto check_1_value {emulated_value};
        check_1_value ^= value2;

        BOOST_TEST(check_1_value == (builtin_value ^ value2));
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

        auto check_1_value {emulated_value};
        check_1_value <<= shift_value;
        BOOST_TEST(check_1_value == (builtin_value << shift_value));

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

        auto check_1_value {emulated_value};
        check_1_value >>= shift_value;
        BOOST_TEST(check_1_value == (builtin_value >> shift_value));

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


        auto check_1_value {emulated_value};
        check_1_value += value2;
        BOOST_TEST(check_1_value == (builtin_value + value2));
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

        auto check_1_value {emulated_value};
        check_1_value -= value2;
        BOOST_TEST(check_1_value == (builtin_value - value2));
        BOOST_TEST((value2 - emulated_value) == (value2 - builtin_value));
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

        auto check_1_value {emulated_value};
        check_1_value *= value2;
        BOOST_TEST(check_1_value == (builtin_value * value2));
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

        auto check_1_value {emulated_value};
        check_1_value *= value2;
        BOOST_TEST(check_1_value == (builtin_value * value2));
        BOOST_TEST((value2 * emulated_value) == (value2 * builtin_value));
    }
}

template <typename IntType>
void test_operator_div()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        IntType value {0};
        IntType value2 {0};

        while (value == 0)
        {
            value = dist(rng);
        }
        while (value2 == 0)
        {
            value2 = dist(rng);
        }

        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        auto check_1_value {emulated_value};
        check_1_value /= value2;
        BOOST_TEST(check_1_value == (builtin_value / value2));
        BOOST_TEST((value2 / emulated_value) == (value2 / builtin_value));
    }
}

template <typename IntType>
void test_operator_mod()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
                                                          std::numeric_limits<IntType>::max());

    for (std::size_t i {}; i < N; ++i)
    {
        IntType value {0};
        IntType value2 {0};

        while (value == 0)
        {
            value = dist(rng);
        }
        while (value2 == 0)
        {
            value2 = dist(rng);
        }

        unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
        boost::decimal::detail::u128 emulated_value {value};

        auto check_1_value {emulated_value};
        check_1_value %= value2;
        BOOST_TEST(check_1_value == (builtin_value % value2));
        BOOST_TEST((value2 % emulated_value) == (value2 % builtin_value));
    }
}

template <typename IntType>
void test_spot_operator_div(IntType value, IntType value2)
{
    unsigned __int128 builtin_value = static_cast<unsigned __int128>(value);
    boost::decimal::detail::u128 emulated_value {value};

    BOOST_TEST((emulated_value / value2) == (builtin_value / value2));
    BOOST_TEST((value2 / emulated_value) == (value2 / builtin_value));
}

void test_ostream_operator()
{
    {
        std::stringstream out;
        constexpr boost::decimal::detail::u128 small_num {0, 15};
        out << small_num;
        BOOST_TEST_CSTR_EQ(out.str().c_str(), "15");

        std::stringstream out_hex_upper;
        std::stringstream out_hex_lower;
        out_hex_upper << std::hex << std::uppercase << small_num;
        out_hex_lower << std::hex << small_num;
        BOOST_TEST_CSTR_EQ(out_hex_upper.str().c_str(), "F");
        BOOST_TEST_CSTR_EQ(out_hex_lower.str().c_str(), "f");

        std::stringstream out_oct;
        out_oct << std::oct << small_num;
        BOOST_TEST_CSTR_EQ(out_oct.str().c_str(), "17");
    }

    {
        std::stringstream out;
        constexpr boost::decimal::detail::u128 big_num {0xF, 0};

        out << big_num;
        BOOST_TEST_CSTR_EQ(out.str().c_str(), "276701161105643274240");

        std::stringstream out_hex_upper;
        std::stringstream out_hex_lower;
        out_hex_upper << std::hex << std::uppercase << big_num;
        out_hex_lower << std::hex << big_num;
        BOOST_TEST_CSTR_EQ(out_hex_upper.str().c_str(), "F0000000000000000");
        BOOST_TEST_CSTR_EQ(out_hex_lower.str().c_str(), "f0000000000000000");

        std::stringstream out_oct;
        out_oct << std::oct << big_num;
        BOOST_TEST_CSTR_EQ(out_oct.str().c_str(), "36000000000000000000000");
    }
}

int main()
{
    test_traits();

    test_numeric_limits();

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

    test_operator_div<std::int8_t>();
    test_operator_div<std::int16_t>();
    test_operator_div<std::int32_t>();
    test_operator_div<std::int64_t>();
    test_operator_div<__int128>();

    test_operator_div<std::uint8_t>();
    test_operator_div<std::uint16_t>();
    test_operator_div<std::uint32_t>();
    test_operator_div<std::uint64_t>();
    test_operator_div<unsigned __int128>();

    test_spot_operator_div(1, -94);

    test_operator_mod<std::int8_t>();
    test_operator_mod<std::int16_t>();
    test_operator_mod<std::int32_t>();
    test_operator_mod<std::int64_t>();
    test_operator_mod<__int128>();

    test_operator_mod<std::uint8_t>();
    test_operator_mod<std::uint16_t>();
    test_operator_mod<std::uint32_t>();
    test_operator_mod<std::uint64_t>();
    test_operator_mod<unsigned __int128>();

    test_ostream_operator();

    return boost::report_errors();
}

#else

using boost::decimal::detail::uint128;

template <typename IntType>
void test_arithmetic_constructor()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        uint128 builtin_value { value };
        boost::decimal::detail::u128 emulated_value{ value };

        BOOST_TEST(builtin_value.high == emulated_value.high && builtin_value.low == emulated_value.low);
    }
}

template <typename IntType>
void test_assignment_operators()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        uint128 builtin_value;
        builtin_value = value;
        boost::decimal::detail::u128 emulated_value{};
        emulated_value = value;

        BOOST_TEST(builtin_value.high == emulated_value.high && builtin_value.low == emulated_value.low);
    }
}

template <typename IntType>
void test_integer_conversion_operators()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        uint128 builtin_value;
        builtin_value = value;
        boost::decimal::detail::u128 emulated_value{};
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

    for (std::size_t i{}; i < N; ++i)
    {
        const auto value{ dist(rng) };
        uint128 builtin_value;
        builtin_value = value;
        boost::decimal::detail::u128 emulated_value{};
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

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        uint128 builtin_value = value;
        boost::decimal::detail::u128 emulated_value{ value };
        emulated_value = +emulated_value;

        BOOST_TEST(builtin_value.high == emulated_value.high && builtin_value.low == builtin_value.low);
    }
}

template <typename IntType = std::uint64_t>
void test_unary_minus()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        uint128 builtin_value = value;
        builtin_value = -builtin_value;
        boost::decimal::detail::u128 emulated_value{ value };
        emulated_value = -emulated_value;

        BOOST_TEST(builtin_value.high == emulated_value.high && builtin_value.low == builtin_value.low);
    }
}

template <typename IntType>
void test_operator_equality()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    // Always equal
    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        BOOST_TEST(((value == emulated_value) == (emulated_value == value)) == (value == value));
    }

    // Potentially equal
    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        BOOST_TEST(((value2 == emulated_value) == (value2 == value)) ==
                   ((emulated_value == value2) == (value == value2)));
    }

    boost::decimal::detail::u128 bool_val{ dist(rng) };
    BOOST_TEST((true == bool_val) == (bool_val == true));
}

template <typename IntType>
void test_operator_inequality()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    // Always equal
    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        BOOST_TEST(((value != emulated_value) == (emulated_value != value)) == !(value != value));
    }

    // Potentially equal
    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        BOOST_TEST(((value2 != emulated_value) == (value2 != value)) ==
                   ((emulated_value != value2) == (value != value2)));
    }

    boost::decimal::detail::u128 bool_val{ dist(rng) };
    BOOST_TEST((true == bool_val) == (bool_val == true));
}

template <typename IntType>
void test_operator_less()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        if (value2 >= 0)
        {
            BOOST_TEST(((value2 < emulated_value) == (value2 < value)) ==
                       ((emulated_value < value2) == (value < value2)));
        }
        else if (value2 < value)
        {
            BOOST_TEST((value2 < emulated_value) == (value2 < value));
        }
        else
        {
            BOOST_TEST((value2 < emulated_value) != (value2 < value));
        }
    }
}

template <typename IntType>
void test_operator_le()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        if (value2 >= 0)
        {
            BOOST_TEST(((value2 <= emulated_value) == (value2 <= value)) ==
                ((emulated_value <= value2) == (value <= value2)));
        }
        else if (value2 <= value)
        {
            BOOST_TEST((value2 <= emulated_value) == (value2 <= value));
        }
        else
        {
            BOOST_TEST((value2 <= emulated_value) != (value2 <= value));
        }
    }
}

template <typename IntType>
void test_operator_greater()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        if (value2 >= 0)
        {
            BOOST_TEST(((value2 > emulated_value) == (value2 > value)) ==
                ((emulated_value > value2) == (value > value2)));
        }
        else if (value2 > value)
        {
            BOOST_TEST((value2 > emulated_value) != (value2 > value));
        }
        else
        {
            BOOST_TEST((value2 > emulated_value) == (value2 > value));
        }
    }
}

template <typename IntType>
void test_operator_ge()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        if (value2 >= 0)
        {
            BOOST_TEST(((value2 >= emulated_value) == (value2 >= value)) ==
                ((emulated_value >= value2) == (value >= value2)));
        }
        else if (value2 >= value)
        {
            BOOST_TEST((value2 >= emulated_value) != (value2 >= value));
        }
        else
        {
            BOOST_TEST((value2 >= emulated_value) == (value2 >= value));
        }
    }
}

template <typename IntType>
void test_operator_not()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        uint128 builtin_value{ value };
        builtin_value = ~builtin_value;
        boost::decimal::detail::u128 emulated_value{ value };
        emulated_value = ~emulated_value;

        BOOST_TEST(builtin_value.high == emulated_value.high && builtin_value.low == emulated_value.low);
    }
}


template <typename IntType>
void test_operator_and()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        BOOST_TEST((emulated_value.low & value2) == (value & value2));
        BOOST_TEST((value2 & value) == (value2 & emulated_value.low));
    }
}

template <typename IntType>
void test_operator_xor()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        boost::decimal::detail::u128 emulated_value{ value };

        BOOST_TEST((emulated_value.low | value2) == (value | value2));
        BOOST_TEST((value2 | value) == (value2 | emulated_value.low));
    }
}


template <typename IntType>
void test_operator_add()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        uint128 builtin_value{ value };
        boost::decimal::detail::u128 emulated_value{ value };

        const auto builtin_res_left = builtin_value + value2;
        const auto builtin_res_right = value2 + builtin_value;

        const auto emulated_res_left = emulated_value + value2;
        const auto emulated_res_right = value2 + emulated_value;

        BOOST_TEST(emulated_res_left.high == builtin_res_left.high && emulated_res_left.low == builtin_res_left.low);
        BOOST_TEST(emulated_res_right.high == builtin_res_right.high && emulated_res_right.low == builtin_res_right.low);
    }
}

template <typename IntType>
void test_operator_sub()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        uint128 builtin_value{ value };
        boost::decimal::detail::u128 emulated_value{ value };

        // uint128 does not have all direction subtraction operators implemented

        //const auto builtin_res_left = builtin_value - value2;
        const auto builtin_res_right = value2 - builtin_value;

        //const auto emulated_res_left = emulated_value - value2;
        const auto emulated_res_right = value2 - emulated_value;

        //BOOST_TEST(emulated_res_left.high == builtin_res_left.high && emulated_res_left.low == builtin_res_left.low);
        BOOST_TEST(emulated_res_right.high == builtin_res_right.high && emulated_res_right.low == builtin_res_right.low);
    }
}

template <typename IntType>
void test_operator_mul()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(), std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        const IntType value2{ dist(rng) };
        uint128 builtin_value{ value };
        boost::decimal::detail::u128 emulated_value{ value };

        
        const auto builtin_res_left = builtin_value * static_cast<uint128>(value2);
        const auto emulated_res_left = emulated_value * static_cast<boost::decimal::detail::u128>(value2);

        BOOST_TEST(emulated_res_left.high == builtin_res_left.high && emulated_res_left.low == builtin_res_left.low);

    }
}

template <typename IntType>
void test_operator_div()
{
    boost::random::uniform_int_distribution<IntType> dist(std::numeric_limits<IntType>::min(), std::numeric_limits<IntType>::max());

    for (std::size_t i{}; i < N; ++i)
    {
        const IntType value{ dist(rng) };
        IntType value2{ dist(rng) };

        while (value2 == 0)
        {
            value2 = dist(rng);
        }

        uint128 builtin_value{ static_cast<std::uint64_t>(value), static_cast<std::uint64_t>(value) };
        boost::decimal::detail::u128 emulated_value{ static_cast<std::uint64_t>(value), static_cast<std::uint64_t>(value) };


        const auto builtin_res_left = builtin_value / static_cast<uint128>(value2);
        const auto emulated_res_left = emulated_value / static_cast<boost::decimal::detail::u128>(value2);

        BOOST_TEST(emulated_res_left.high == builtin_res_left.high && emulated_res_left.low == builtin_res_left.low);

    }
}

int main()
{
    test_traits();

    test_numeric_limits();

    test_arithmetic_constructor<std::int8_t>();
    test_arithmetic_constructor<std::int16_t>();
    test_arithmetic_constructor<std::int32_t>();
    test_arithmetic_constructor<std::int64_t>();

    test_arithmetic_constructor<std::uint8_t>();
    test_arithmetic_constructor<std::uint16_t>();
    test_arithmetic_constructor<std::uint32_t>();
    test_arithmetic_constructor<std::uint64_t>();

    test_assignment_operators<std::int8_t>();
    test_assignment_operators<std::int16_t>();
    test_assignment_operators<std::int32_t>();
    test_assignment_operators<std::int64_t>();

    test_assignment_operators<std::uint8_t>();
    test_assignment_operators<std::uint16_t>();
    test_assignment_operators<std::uint32_t>();
    test_assignment_operators<std::uint64_t>();

    test_integer_conversion_operators<std::int8_t>();
    test_integer_conversion_operators<std::int16_t>();
    test_integer_conversion_operators<std::int32_t>();
    test_integer_conversion_operators<std::int64_t>();

    test_integer_conversion_operators<std::uint8_t>();
    test_integer_conversion_operators<std::uint16_t>();
    test_integer_conversion_operators<std::uint32_t>();
    test_integer_conversion_operators<std::uint64_t>();

    test_float_conversion_operators<float>();
    test_float_conversion_operators<double>();
    test_float_conversion_operators<long double>();

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

    test_operator_inequality<std::int8_t>();
    test_operator_inequality<std::int16_t>();
    test_operator_inequality<std::int32_t>();
    test_operator_inequality<std::int64_t>();

    test_operator_inequality<std::uint8_t>();
    test_operator_inequality<std::uint16_t>();
    test_operator_inequality<std::uint32_t>();
    test_operator_inequality<std::uint64_t>();

    test_operator_less<std::int8_t>();
    test_operator_less<std::int16_t>();
    test_operator_less<std::int32_t>();
    test_operator_less<std::int64_t>();

    test_operator_less<std::uint8_t>();
    test_operator_less<std::uint16_t>();
    test_operator_less<std::uint32_t>();
    test_operator_less<std::uint64_t>();

    test_operator_le<std::int8_t>();
    test_operator_le<std::int16_t>();
    test_operator_le<std::int32_t>();
    test_operator_le<std::int64_t>();

    test_operator_le<std::uint8_t>();
    test_operator_le<std::uint16_t>();
    test_operator_le<std::uint32_t>();
    test_operator_le<std::uint64_t>();

    test_operator_greater<std::int8_t>();
    test_operator_greater<std::int16_t>();
    test_operator_greater<std::int32_t>();
    test_operator_greater<std::int64_t>();

    test_operator_greater<std::uint8_t>();
    test_operator_greater<std::uint16_t>();
    test_operator_greater<std::uint32_t>(); 
    test_operator_greater<std::uint64_t>();

    test_operator_ge<std::int8_t>();
    test_operator_ge<std::int16_t>();
    test_operator_ge<std::int32_t>();
    test_operator_ge<std::int64_t>();

    test_operator_ge<std::uint8_t>();
    test_operator_ge<std::uint16_t>();
    test_operator_ge<std::uint32_t>();
    test_operator_ge<std::uint64_t>();

    test_operator_not<std::int8_t>();
    test_operator_not<std::int16_t>();
    test_operator_not<std::int32_t>();
    test_operator_not<std::int64_t>();

    test_operator_not<std::uint8_t>();
    test_operator_not<std::uint16_t>();
    test_operator_not<std::uint32_t>();
    test_operator_not<std::uint64_t>();

    test_operator_and<std::int8_t>();
    test_operator_and<std::int16_t>();
    test_operator_and<std::int32_t>();
    test_operator_and<std::int64_t>();

    test_operator_and<std::uint8_t>();
    test_operator_and<std::uint16_t>();
    test_operator_and<std::uint32_t>();
    test_operator_and<std::uint64_t>();

    test_operator_xor<std::int8_t>();
    test_operator_xor<std::int16_t>();
    test_operator_xor<std::int32_t>();
    test_operator_xor<std::int64_t>();

    test_operator_xor<std::uint8_t>();
    test_operator_xor<std::uint16_t>();
    test_operator_xor<std::uint32_t>();
    test_operator_xor<std::uint64_t>();

    test_operator_add<std::int8_t>();
    test_operator_add<std::int16_t>();
    test_operator_add<std::int32_t>();
    test_operator_add<std::int64_t>();

    test_operator_add<std::uint8_t>();
    test_operator_add<std::uint16_t>();
    test_operator_add<std::uint32_t>();
    test_operator_add<std::uint64_t>();

    test_operator_sub<std::int8_t>();
    test_operator_sub<std::int16_t>();
    test_operator_sub<std::int32_t>();
    test_operator_sub<std::int64_t>();

    test_operator_sub<std::uint8_t>();
    test_operator_sub<std::uint16_t>();
    test_operator_sub<std::uint32_t>();
    test_operator_sub<std::uint64_t>();

    test_operator_mul<std::int8_t>();
    test_operator_mul<std::int16_t>();
    test_operator_mul<std::int32_t>();
    test_operator_mul<std::int64_t>();

    test_operator_mul<std::uint8_t>();
    test_operator_mul<std::uint16_t>();
    test_operator_mul<std::uint32_t>();
    test_operator_mul<std::uint64_t>();

    test_operator_div<std::int8_t>();
    test_operator_div<std::int16_t>();
    test_operator_div<std::int32_t>();
    test_operator_div<std::int64_t>();

    test_operator_div<std::uint8_t>();
    test_operator_div<std::uint16_t>();
    test_operator_div<std::uint32_t>();
    test_operator_div<std::uint64_t>();

    return boost::report_errors();
}

#endif
