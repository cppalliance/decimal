// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL128_HPP
#define BOOST_DECIMAL_DECIMAL128_HPP

#include <cassert>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <iostream>
#include <limits>
#include <type_traits>
#include <sstream>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/emulated256.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <boost/decimal/detail/comparison.hpp>
#include <boost/decimal/detail/mixed_decimal_arithmetic.hpp>
#include <boost/decimal/detail/to_integral.hpp>
#include <boost/decimal/detail/to_float.hpp>
#include <boost/decimal/detail/to_decimal.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/io.hpp>
#include <boost/decimal/detail/check_non_finite.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>

namespace boost {
namespace decimal {

namespace detail {

// See IEEE 754 section 3.5.2
static constexpr uint128 d128_inf_mask {UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
static constexpr uint128 d128_nan_mask {UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
static constexpr uint128 d128_snan_mask {UINT64_C(0b0'11111'10000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
static constexpr uint128 d128_comb_inf_mask {UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
static constexpr uint128 d128_comb_nan_mask {UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
static constexpr uint128 d128_exp_snan_mask {UINT64_C(0b0'00000'10000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeeeeeeeee (0TTT) 110-bits
// s 01 TTT (01)eeeeeeeeeeee (0TTT) 110-bits
// s 10 TTT (10)eeeeeeeeeeee (0TTT) 110-bits
static constexpr std::uint64_t d128_significand_bits = UINT64_C(110);
static constexpr std::uint64_t d128_exponent_bits = UINT64_C(12);

static constexpr uint128 d128_sign_mask {UINT64_C(0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                         UINT64_C(0)};
static constexpr uint128 d128_combination_field_mask {UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                      UINT64_C(0)};
static constexpr uint128 d128_exponent_mask {UINT64_C(0b0'00000'111111111111'0000000000'0000000000'0000000000'0000000000'000000),
                                             UINT64_C(0)};

static constexpr uint128 d128_significand_mask {UINT64_C(0b1111111111'1111111111'1111111111'1111111111'111111), UINT64_MAX};

static constexpr uint128 d128_comb_01_mask {UINT64_C(0b0'01000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                            UINT64_C(0)};
static constexpr uint128 d128_comb_10_mask {UINT64_C(0b0'10000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                            UINT64_C(0)};
static constexpr uint128 d128_comb_00_01_10_significand_bits {UINT64_C(0b0'00111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                              UINT64_C(0)};

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
static constexpr uint128 d128_comb_11_mask {UINT64_C(0b0'11000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                            UINT64_C(0)};
static constexpr uint128 d128_comb_11_exp_bits {UINT64_C(0b0'00110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                UINT64_C(0)};
static constexpr uint128 d128_comb_11_significand_bits {UINT64_C(0b0'00001'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                        UINT64_C(0)};

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeeeeeeeee (100T) 110-bits
// s 1101 T (01)eeeeeeeeeeee (100T) 110-bits
// s 1110 T (10)eeeeeeeeeeee (100T) 110-bits
static constexpr uint128 d128_comb_1101_mask {UINT64_C(0b0'11010'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                              UINT64_C(0)};
static constexpr uint128 d128_comb_1110_mask {UINT64_C(0b0'11100'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                              UINT64_C(0)};

// Powers of 2 used to determine the size of the significand
static constexpr uint128 d128_no_combination {d128_significand_mask};
static constexpr uint128 d128_big_combination {UINT64_C(0b111'1111111111'1111111111'1111111111'1111111111'111111),
                                               UINT64_MAX};

// Exponent Fields
static constexpr std::uint64_t d128_max_exp_no_combination {0b111111111111};
static constexpr std::uint64_t d128_exp_one_combination {0b1'111111111111};
static constexpr std::uint64_t d128_max_biased_exp {0b10'111111111111};
static constexpr uint128 d128_small_combination_field_mask {UINT64_C(0b111'0000000000'0000000000'0000000000'0000000000'000000),
                                                            UINT64_C(0)};
static constexpr uint128 d128_big_combination_field_mask {UINT64_C(0b1'0000000000'0000000000'0000000000'0000000000'000000),
                                                          UINT64_C(0)};

struct decimal128_components
{
    uint128 sig;
    std::int32_t exp;
    bool sign;
};

} //namespace detail

class decimal128 final
{
private:
    detail::uint128 bits_ {};

    #ifdef BOOST_DECIMAL_HAS_INT128

    friend constexpr auto from_bits(detail::uint128_t rhs) noexcept -> decimal128;
    friend constexpr auto to_bits(decimal128 rhs) noexcept -> detail::uint128_t;

    #endif

    friend constexpr auto from_bits(detail::uint128 rhs) noexcept -> decimal128;

    constexpr auto unbiased_exponent() const noexcept -> std::uint64_t;
    constexpr auto biased_exponent() const noexcept -> std::int32_t;
    constexpr auto full_significand() const noexcept -> detail::uint128;
    constexpr auto isneg() const noexcept -> bool;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral_128(Decimal val) noexcept -> TargetType;

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept -> TargetType;

    template <typename TargetType, typename Decimal>
    friend constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    // Equality template between any integer type and decimal128
    template <typename Decimal, typename Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <typename Decimal1, typename Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal128
    template <typename Decimal, typename Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <typename Decimal1, typename Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    template <typename T1, typename T2>
    constexpr auto d128_add_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                 T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept
                                 -> detail::decimal128_components;
public:
    // 3.2.4.1 construct/copy/destroy
    constexpr decimal128() noexcept = default;
    constexpr decimal128& operator=(const decimal128& rhs) noexcept = default;
    constexpr decimal128(const decimal128& rhs) noexcept = default;

    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal128(Float val) noexcept;

    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    explicit constexpr decimal128(Integer val) noexcept;

    // 3.2.5 initialization from coefficient and exponent:
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1>, bool> = true>
    constexpr decimal128(T1 coeff, T2 exp, bool sign = false) noexcept;

    // 3.2.4.4 Conversion to integral type
    explicit constexpr operator int() const noexcept;
    explicit constexpr operator unsigned() const noexcept;
    explicit constexpr operator long() const noexcept;
    explicit constexpr operator unsigned long() const noexcept;
    explicit constexpr operator long long() const noexcept;
    explicit constexpr operator unsigned long long() const noexcept;
    explicit constexpr operator std::int8_t() const noexcept;
    explicit constexpr operator std::uint8_t() const noexcept;
    explicit constexpr operator std::int16_t() const noexcept;
    explicit constexpr operator std::uint16_t() const noexcept;

    // 3.2.6 Conversion to floating-point type
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator float() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator double() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator long double() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_FLOAT16
    explicit constexpr operator std::float16_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_FLOAT32
    explicit constexpr operator std::float32_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_FLOAT64
    explicit constexpr operator std::float64_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
    explicit constexpr operator std::bfloat16_t() const noexcept;
    #endif

    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    explicit constexpr operator Decimal() const noexcept;

    // cmath functions that are easier as friends
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool;

    // 3.2.7 unary arithmetic operators:
    friend constexpr auto operator+(decimal128 rhs) noexcept -> decimal128;
    friend constexpr auto operator-(decimal128 rhs) noexcept -> decimal128;

    // 3.2.8 Binary arithmetic operators
    friend constexpr auto operator+(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    // 3.2.9 Comparison operators:
    // Equality
    friend constexpr auto operator==(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal128 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal128 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Inequality
    friend constexpr auto operator!=(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal128 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal128 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less
    friend constexpr auto operator<(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal128 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal128 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less equal
    friend constexpr auto operator<=(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal128 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal128 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater
    friend constexpr auto operator>(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal128 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal128 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater equal
    friend constexpr auto operator>=(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal128 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal128 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // C++20 spaceship
    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal128 lhs, decimal128 rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal128 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>;

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal128 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>;
    #endif

    // 3.2.10 Formatted input:
    template <typename charT, typename traits, typename DecimalType>
    friend auto operator>>(std::basic_istream<charT, traits>& is, DecimalType& d)
        -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_istream<charT, traits>&>;

    // 3.2.11 Formatted output:
    template <typename charT, typename traits, typename DecimalType>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, const DecimalType& d)
        -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_ostream<charT, traits>&>;

    friend std::string bit_string(decimal128 rhs) noexcept;
};

std::string bit_string(decimal128 rhs) noexcept
{
    std::stringstream ss;
    ss << std::hex << rhs.bits_.high << rhs.bits_.low;
    return ss.str();
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr auto from_bits(detail::uint128_t rhs) noexcept -> decimal128
{
    decimal128 result;
    result.bits_ = rhs;

    return result;
}

constexpr auto to_bits(decimal128 rhs) noexcept -> detail::uint128_t
{
    return static_cast<detail::uint128_t>(rhs.bits_);
}

#endif

constexpr auto from_bits(detail::uint128 rhs) noexcept -> decimal128
{
    decimal128 result;
    result.bits_ = rhs;

    return result;
}

constexpr auto decimal128::unbiased_exponent() const noexcept -> std::uint64_t
{
    std::uint64_t expval {};
    constexpr std::uint64_t high_word_significand_bits {detail::d128_significand_bits - 64U};

    const auto exp_comb_bits {(bits_.high & detail::d128_comb_11_mask.high)};

    if (exp_comb_bits == detail::d128_comb_11_mask.high)
    {
        expval = (bits_.high & detail::d128_comb_11_mask.high) >> (high_word_significand_bits + 1);
    }
    else if (exp_comb_bits == detail::d128_comb_10_mask.high)
    {
        expval = UINT64_C(0b10000000000000);
    }
    else if (exp_comb_bits == detail::d128_comb_01_mask.high)
    {
        expval = UINT64_C(0b01000000000000);
    }

    expval |= (bits_.high & detail::d128_exponent_mask.high) >> high_word_significand_bits;

    return expval;
}

constexpr auto decimal128::biased_exponent() const noexcept -> std::int32_t
{
    return static_cast<std::int32_t>(unbiased_exponent()) - detail::bias_v<decimal128>;
}

constexpr auto decimal128::full_significand() const noexcept -> detail::uint128
{
    detail::uint128 significand {0, 0};

    if ((bits_.high & detail::d128_comb_11_mask.high) == detail::d128_comb_11_mask.high)
    {
        // Only need the one bit of T because the other 3 are implied 0s
        if ((bits_.high & detail::d128_comb_11_significand_bits.high) == detail::d128_comb_11_significand_bits.high)
        {
            significand = detail::uint128{0b10010000000000000000000000000000000000000000000000,0};
        }
        else
        {
            significand = detail::uint128{0b10000000000000000000000000000000000000000000000000,0};
        }
    }
    else
    {
        // Last three bits in the combination field, so we need to shift past the exp field
        // which is next. Only need to operate on the high bits
        significand.high |= (bits_.high & detail::d128_comb_00_01_10_significand_bits.high) >> detail::d128_exponent_bits;
    }

    significand |= (bits_ & detail::d128_significand_mask);

    return significand;
}

constexpr auto decimal128::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_.high & detail::d128_sign_mask.high);
}

// TODO(mborland): Rather than doing bitwise operations on the whole uint128 we should
// be able to only operate on the affected word
//
// e.g. for sign bits_.high |= detail::d128_sign_mask.high
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1>, bool>>
constexpr decimal128::decimal128(T1 coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = detail::make_unsigned_t<T1>;

    bits_ = {UINT64_C(0), UINT64_C(0)};
    bool isneg {false};
    Unsigned_Integer unsigned_coeff {detail::make_positive_unsigned(coeff)};
    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<T1>)
    {
        if (coeff < 0 || sign)
        {
            bits_.high = detail::d128_sign_mask.high;
            isneg = true;
        }
    }
    else
    {
        if (sign)
        {
            bits_.high = detail::d128_sign_mask.high;
            isneg = true;
        }
    }

    // If the coeff is not in range make it so
    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision_v<decimal128>};
    while (unsigned_coeff_digits > detail::precision_v<decimal128> + 1)
    {
        unsigned_coeff /= 10;
        ++exp;
        --unsigned_coeff_digits;
    }

    // Round as required
    if (reduced)
    {
        exp += detail::fenv_round<decimal128>(unsigned_coeff, isneg);
    }

    auto reduced_coeff {static_cast<detail::uint128>(unsigned_coeff)};
    bool big_combination {false};

    if (reduced_coeff == detail::uint128{0, 0})
    {
        exp = 0;
    }
    else if (reduced_coeff <= detail::d128_no_combination)
    {
        // If the coefficient fits directly we don't need to use the combination field
        bits_ |= (reduced_coeff & detail::d128_significand_mask);
    }
    else if (reduced_coeff <= detail::d128_big_combination)
    {
        // Break the number into 3 bits for the combination field and 110 bits for the significand field

        // Use the least significant 110 bits to set the significand
        bits_ |= (reduced_coeff & detail::d128_significand_mask);

        // Now set the combination field (maximum of 3 bits)
        auto remaining_bits {reduced_coeff & detail::d128_small_combination_field_mask};
        remaining_bits <<= detail::d128_exponent_bits;
        bits_ |= remaining_bits;
    }
    else
    {
        // Have to use the full combination field
        bits_ |= detail::d128_comb_11_mask;
        big_combination = true;

        bits_ |= (reduced_coeff & detail::d128_significand_mask);
        const auto remaining_bit {reduced_coeff & detail::d128_big_combination_field_mask};

        if (remaining_bit)
        {
            bits_ |= detail::d128_comb_11_significand_bits;
        }
    }

    // If the exponent fits we do not need to use the combination field
    auto biased_exp {static_cast<std::uint64_t>(exp + detail::bias_v<decimal128>)};
    const auto biased_exp_low_twelve_bits {detail::uint128(biased_exp & detail::d128_max_exp_no_combination) <<
                                           detail::d128_significand_bits};

    if (biased_exp <= detail::d128_max_exp_no_combination)
    {
        bits_ |= biased_exp_low_twelve_bits;
    }
    else if (biased_exp <= detail::d128_exp_one_combination)
    {
        if (big_combination)
        {
            bits_ |= (detail::d128_comb_1101_mask | biased_exp_low_twelve_bits);
        }
        else
        {
            bits_ |= (detail::d128_comb_01_mask | biased_exp_low_twelve_bits);
        }
    }
    else if (biased_exp <= detail::d128_max_biased_exp)
    {
        if (big_combination)
        {
            bits_ |= (detail::d128_comb_1110_mask | biased_exp_low_twelve_bits);
        }
        else
        {
            bits_ |= (detail::d128_comb_10_mask | biased_exp_low_twelve_bits);
        }
    }
    else
    {
        // The value is probably infinity

        // If we can offset some extra power in the coefficient try to do so
        const auto coeff_dig {detail::num_digits(reduced_coeff)};
        if (coeff_dig < detail::precision_v<decimal128>)
        {
            for (auto i {coeff_dig}; i <= detail::precision_v<decimal128>; ++i)
            {
                reduced_coeff *= 10;
                --biased_exp;
                --exp;
                if (biased_exp == detail::d128_max_biased_exp)
                {
                    break;
                }
            }

            if (detail::num_digits(reduced_coeff) <= detail::precision_v<decimal128>)
            {
                *this = decimal128(reduced_coeff, exp, isneg);
            }
            else
            {
                if (exp < 0)
                {
                    *this = decimal128(0, 0, isneg);
                }
                else
                {
                    bits_ = detail::d128_comb_inf_mask;
                }
            }
        }
        else
        {
            bits_ = detail::d128_comb_inf_mask;
        }
    }
}

template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
BOOST_DECIMAL_CXX20_CONSTEXPR decimal128::decimal128(Float val) noexcept
{
    if (val != val)
    {
        *this = from_bits(detail::d128_nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        *this = from_bits(detail::d128_inf_mask);
    }
    else
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};

        #ifdef BOOST_DECIMAL_DEBUG
        std::cerr << "Mant: " << components.mantissa
                  << "\nExp: " << components.exponent
                  << "\nSign: " << components.sign << std::endl;
        #endif

        if (components.exponent > detail::emax_v<decimal128>)
        {
            *this = from_bits(detail::d128_inf_mask);
        }
        else
        {
            *this = decimal128 {components.mantissa, components.exponent, components.sign};
        }
    }
}

template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
constexpr decimal128::decimal128(Integer val) noexcept // NOLINT : Incorrect parameter is never used
{
    *this = decimal128{val, 0};
}

constexpr decimal128::operator int() const noexcept
{
    return to_integral_128<decimal128, int>(*this);
}

constexpr decimal128::operator unsigned() const noexcept
{
    return to_integral_128<decimal128, unsigned>(*this);
}

constexpr decimal128::operator long() const noexcept
{
    return to_integral_128<decimal128, long>(*this);
}

constexpr decimal128::operator unsigned long() const noexcept
{
    return to_integral_128<decimal128, unsigned long>(*this);
}

constexpr decimal128::operator long long() const noexcept
{
    return to_integral_128<decimal128, long long>(*this);
}

constexpr decimal128::operator unsigned long long() const noexcept
{
    return to_integral_128<decimal128, unsigned long long>(*this);
}

constexpr decimal128::operator std::int8_t() const noexcept
{
    return to_integral_128<decimal128, std::int8_t>(*this);
}

constexpr decimal128::operator std::uint8_t() const noexcept
{
    return to_integral_128<decimal128, std::uint8_t>(*this);
}

constexpr decimal128::operator std::int16_t() const noexcept
{
    return to_integral_128<decimal128, std::int16_t>(*this);
}

constexpr decimal128::operator std::uint16_t() const noexcept
{
    return to_integral_128<decimal128, std::uint16_t>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128::operator float() const noexcept
{
    return to_float<decimal128, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128::operator double() const noexcept
{
    return to_float<decimal128, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128::operator long double() const noexcept
{
    return to_float<decimal128, long double>(*this);
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal128::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal128, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal128::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal128, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal128::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal128, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal128::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal128, float>(*this));
}
#endif

template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal128::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool
{
    return rhs.bits_.high & detail::d128_sign_mask.high;
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool
{
    return (rhs.bits_.high & detail::d128_nan_mask.high) == detail::d128_nan_mask.high;
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool
{
    return ((rhs.bits_.high & detail::d128_nan_mask.high) == detail::d128_inf_mask.high);
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool
{
    return (rhs.bits_.high & detail::d128_snan_mask.high) == detail::d128_snan_mask.high;
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128 rhs) noexcept -> bool
{
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision_v<decimal128> - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
}

constexpr auto operator+(decimal128 rhs) noexcept -> decimal128
{
    return rhs;
}

constexpr auto operator-(decimal128 rhs) noexcept-> decimal128
{
    rhs.bits_.high ^= detail::d128_sign_mask.high;
    return rhs;
}


constexpr auto operator==(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    // Check for IEEE requirement that nan != nan
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return equal_parts_impl<decimal128>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                        rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator==(decimal128 lhs, Integer rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal128 rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal128 lhs, Integer rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal128 rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs) ||
        (!lhs.isneg() && rhs.isneg()))
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
    else if (isfinite(lhs) && isinf(rhs))
    {
        if (!rhs.isneg())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return less_parts_impl<decimal128>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                       rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator<(decimal128 lhs, Integer rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal128 rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !less_impl(rhs, lhs) && lhs != rhs;
}

constexpr auto operator<=(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(decimal128 lhs, Integer rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(lhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal128 rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

constexpr auto operator>(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal128 lhs, Integer rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal128 rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(decimal128 lhs, Integer rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(lhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal128 rhs) noexcept
-> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

constexpr auto operator<=>(decimal128 lhs, decimal128 rhs) noexcept -> std::partial_ordering
{
    if (lhs < rhs)
    {
        return std::partial_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::partial_ordering::greater;
    }
    else if (lhs == rhs)
    {
        return std::partial_ordering::equivalent;
    }

    return std::partial_ordering::unordered;
}

template <typename Integer>
constexpr auto operator<=>(decimal128 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>
{
    if (lhs < rhs)
    {
        return std::partial_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::partial_ordering::greater;
    }
    else if (lhs == rhs)
    {
        return std::partial_ordering::equivalent;
    }

    return std::partial_ordering::unordered;
}

template <typename Integer>
constexpr auto operator<=>(Integer lhs, decimal128 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>
{
    if (lhs < rhs)
    {
        return std::partial_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::partial_ordering::greater;
    }
    else if (lhs == rhs)
    {
        return std::partial_ordering::equivalent;
    }

    return std::partial_ordering::unordered;
}

#endif

#ifdef BOOST_DECIMAL_DEBUG_ADD_128
static char* mini_to_chars( char (&buffer)[ 64 ], boost::decimal::detail::uint128_t v )
{
    char* p = buffer + 64;
    *--p = '\0';

    do
    {
        *--p = "0123456789"[ v % 10 ];
        v /= 10;
    }
    while ( v != 0 );

    return p;
}

std::ostream& operator<<( std::ostream& os, boost::decimal::detail::uint128_t v )
{
    char buffer[ 64 ];

    os << mini_to_chars( buffer, v );
    return os;
}
#endif

template <typename T1, typename T2>
constexpr auto d128_add_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                             T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal128_components
{
    const bool sign {lhs_sign};

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    if (delta_exp > detail::precision_v<decimal128> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return {lhs_sig, lhs_exp, lhs_sign};
    }
    else if (delta_exp == detail::precision_v<decimal128> + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        BOOST_DECIMAL_IF_CONSTEXPR (std::numeric_limits<T2>::digits10 > std::numeric_limits<std::uint64_t>::digits10)
        {
            if (rhs_sig >= detail::uint128 {500'000'000'000'000, 0})
            {
                ++lhs_sig;
            }

            return {lhs_sig, lhs_exp, lhs_sign};
        }
        else
        {
            return {lhs_sig, lhs_exp, lhs_sign};
        }
    }

    // The two numbers can be added together without special handling
    //
    // If we can add to the lhs sig rather than dividing we can save some precision
    // 64-bit sign int can have 19 digits, and our normalized significand has 16

    if (delta_exp <= 3)
    {
        while (delta_exp > 0)
        {
            lhs_sig *= 10;
            --delta_exp;
            --lhs_exp;
        }
    }
    else
    {
        lhs_sig *= 1000;
        delta_exp -= 3;
        lhs_exp -= 3;
    }

    while (delta_exp > 1)
    {
        rhs_sig /= 10;
        --delta_exp;
    }

    if (delta_exp == 1)
    {
        detail::fenv_round<decimal128>(rhs_sig, rhs_sign);
    }

    // Convert both of the significands to unsigned types, so we can use intrinsics
    // in the uint128 implementation
    const auto unsigned_lhs_sig {detail::make_positive_unsigned(lhs_sig)};
    const auto unsigned_rhs_sig {detail::make_positive_unsigned(rhs_sig)};
    const auto new_sig {static_cast<detail::uint128>(unsigned_lhs_sig + unsigned_rhs_sig)};
    const auto new_exp {lhs_exp};

    #ifdef BOOST_DECIMAL_DEBUG_ADD_128
    std::cerr << "Res Sig: " << static_cast<detail::uint128_t>(new_sig)
              << "\nRes Exp: " << new_exp
              << "\nRes Neg: " << sign << std::endl;
    #endif

    return {new_sig, new_exp, sign};
}

constexpr auto operator+(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    constexpr decimal128 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }

    bool lhs_bigger {lhs > rhs};
    if (lhs.isneg() && rhs.isneg())
    {
        lhs_bigger = !lhs_bigger;
    }

    // Ensure that lhs is always the larger for ease of impl
    if (!lhs_bigger)
    {
        detail::swap(lhs, rhs);
    }

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs - abs(rhs);
    }

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal128>(lhs_sig, lhs_exp);

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal128>(rhs_sig, rhs_exp);

    #ifdef BOOST_DECIMAL_DEBUG_ADD_128
    std::cerr << "\nlhs sig: " << static_cast<detail::uint128_t>(lhs_sig)
              << "\nlhs exp: " << lhs_exp
              << "\nrhs sig: " << static_cast<detail::uint128_t>(rhs_sig)
              << "\nrhs exp: " << rhs_exp << std::endl;
    #endif

    const auto result {d128_add_impl(lhs_sig, lhs_exp, lhs.isneg(),
                                     rhs_sig, rhs_exp, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename charT, typename traits>
auto operator<<(std::basic_ostream<charT, traits>& os, const decimal128& d) -> std::basic_ostream<charT, traits>&
{
    if (d.isneg())
    {
        os << "-";
    }

    #ifdef BOOST_DECIMAL_DEBUG_ADD_128
    os << static_cast<detail::uint128_t>(d.full_significand());
    #else
    os << d.full_significand();
    #endif
    os << "e";

    if (d.biased_exponent() < 0)
    {
        os << d.biased_exponent();
    }
    else
    {
        os << "+" << d.biased_exponent();
    }

    return os;
}

} //namespace decimal
} //namespace boost

namespace std {

template<>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128>
#else
struct numeric_limits<boost::decimal::decimal128>
#endif
{

#ifdef _MSC_VER
    public:
#endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_specialized = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_signed = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_integer = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_exact = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_infinity = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_quiet_NaN = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_signaling_NaN = true;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_denorm_loss = true;
    #endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_round_style round_style = std::round_indeterminate;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_iec559 = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_bounded = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_modulo = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  digits = 34;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  radix = 10;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  min_exponent = -6142;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  min_exponent10 = min_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_exponent = 6145;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_exponent10 = max_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool traps = numeric_limits<std::uint64_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = true;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (min)        () -> boost::decimal::decimal128 { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (max)        () -> boost::decimal::decimal128 { return {boost::decimal::detail::uint128{UINT64_C(999'999'999'999'999), UINT64_C(9'999'999'999'999'999'999)}, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto lowest       () -> boost::decimal::decimal128 { return {boost::decimal::detail::uint128{UINT64_C(999'999'999'999'999), UINT64_C(9'999'999'999'999'999'999)}, max_exponent, true}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto epsilon      () -> boost::decimal::decimal128 { return {1, -34}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto round_error  () -> boost::decimal::decimal128 { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto infinity     () -> boost::decimal::decimal128 { return boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto quiet_NaN    () -> boost::decimal::decimal128 { return boost::decimal::from_bits(boost::decimal::detail::d128_nan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto signaling_NaN() -> boost::decimal::decimal128 { return boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto denorm_min   () -> boost::decimal::decimal128 { return {1, boost::decimal::detail::etiny_v<boost::decimal::decimal128>}; }
};

} //namespace std

#endif //BOOST_DECIMAL_DECIMAL128_HPP
