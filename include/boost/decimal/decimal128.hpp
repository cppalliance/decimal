// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL128_HPP
#define BOOST_DECIMAL_DECIMAL128_HPP

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
#include <boost/decimal/detail/check_non_finite.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <boost/decimal/detail/add_impl.hpp>
#include <boost/decimal/detail/sub_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/div_impl.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
#include <cwchar>
#include <iostream>
#include <sstream>
#endif

#endif //BOOST_DECIMAL_BUILD_MODULE

namespace boost {
namespace decimal {

namespace detail {

// See IEEE 754 section 3.5.2
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_inf_mask {UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_nan_mask {UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_snan_mask {UINT64_C(0b0'11111'10000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_inf_mask {UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_nan_mask {UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_exp_snan_mask {UINT64_C(0b0'00000'10000000'0000000000'0000000000'0000000000'0000000000'0000000000), UINT64_C(0)};

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeeeeeeeee (0TTT) 110-bits
// s 01 TTT (01)eeeeeeeeeeee (0TTT) 110-bits
// s 10 TTT (10)eeeeeeeeeeee (0TTT) 110-bits
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d128_significand_bits = UINT64_C(110);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d128_exponent_bits = UINT64_C(12);

BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_sign_mask {UINT64_C(0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                         UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_combination_field_mask {UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                      UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_exponent_mask {UINT64_C(0b0'00000'111111111111'0000000000'0000000000'0000000000'0000000000'000000),
                                             UINT64_C(0)};

BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_significand_mask {UINT64_C(0b1111111111'1111111111'1111111111'1111111111'111111), UINT64_MAX};

BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_01_mask {UINT64_C(0b0'01000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                            UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_10_mask {UINT64_C(0b0'10000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                            UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_00_01_10_significand_bits {UINT64_C(0b0'00111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                              UINT64_C(0)};

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_11_mask {UINT64_C(0b0'11000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                            UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_11_exp_bits {UINT64_C(0b0'00110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_11_significand_bits {UINT64_C(0b0'00001'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                                        UINT64_C(0)};

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeeeeeeeee (100T) 110-bits
// s 1101 T (01)eeeeeeeeeeee (100T) 110-bits
// s 1110 T (10)eeeeeeeeeeee (100T) 110-bits
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_1101_mask {UINT64_C(0b0'11010'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                              UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_comb_1110_mask {UINT64_C(0b0'11100'00000000'0000000000'0000000000'0000000000'0000000000'0000000000),
                                              UINT64_C(0)};

// Powers of 2 used to determine the size of the significand
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_no_combination {d128_significand_mask};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_big_combination {UINT64_C(0b111'1111111111'1111111111'1111111111'1111111111'111111),
                                               UINT64_MAX};

// Exponent Fields
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d128_max_exp_no_combination {0b111111111111};
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d128_exp_one_combination {0b1'111111111111};
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d128_max_biased_exp {0b10'111111111111};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_small_combination_field_mask {UINT64_C(0b111'0000000000'0000000000'0000000000'0000000000'000000),
                                                            UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE uint128 d128_big_combination_field_mask {UINT64_C(0b1'0000000000'0000000000'0000000000'0000000000'000000),
                                                          UINT64_C(0)};

struct decimal128_components
{
    using sig_type = uint128;

    uint128 sig {};
    std::int32_t exp {};
    bool sign {};

    constexpr decimal128_components() = default;
    constexpr decimal128_components(const decimal128_components& rhs) = default;
    constexpr decimal128_components& operator=(const decimal128_components& rhs) = default;
    constexpr decimal128_components(uint128 sig_, std::int32_t exp_, bool sign_) : sig{sig_}, exp{exp_}, sign{sign_} {}
};

} //namespace detail

BOOST_DECIMAL_EXPORT class decimal128 final
{
public:
    using significand_type = detail::uint128;

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

    // Allows direct editing of the exp
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr auto edit_exponent(T exp) noexcept -> void;
    constexpr auto edit_sign(bool sign) noexcept -> void;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral_128(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_integral_v, TargetType, TargetType);

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_floating_point_v, TargetType, TargetType);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetType, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    friend constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    // Equality template between any integer type and decimal128
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal128
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    friend constexpr auto d128_div_impl(decimal128 lhs, decimal128 rhs, decimal128& q, decimal128& r) noexcept -> void;

    friend constexpr auto d128_mod_impl(decimal128 lhs, decimal128 rhs, const decimal128& q, decimal128& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

public:
    // 3.2.4.1 construct/copy/destroy
    constexpr decimal128() noexcept = default;
    constexpr decimal128& operator=(const decimal128& rhs) noexcept = default;
    constexpr decimal128& operator=(decimal128&& rhs) noexcept = default;
    constexpr decimal128(const decimal128& rhs) noexcept = default;
    constexpr decimal128(decimal128&& rhs) noexcept = default;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    #ifndef BOOST_DECIMAL_ALLOW_IMPLICIT_CONVERSIONS
    explicit
    #endif
    BOOST_DECIMAL_CXX20_CONSTEXPR decimal128(Float val) noexcept;

    template <typename Float>
    BOOST_DECIMAL_CXX20_CONSTEXPR auto operator=(const Float& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal128&);

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    #else
    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    #endif
    explicit constexpr decimal128(Decimal val) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal128(Integer val) noexcept;

    template <typename Integer>
    constexpr auto operator=(const Integer& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&);

    // 3.2.5 initialization from coefficient and exponent:
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal128(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T>
    #else
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    #endif
    constexpr decimal128(bool coeff, T exp, bool sign = false) noexcept;

    // 3.2.4.4 Conversion to integral type
    explicit constexpr operator bool() const noexcept;
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

    explicit constexpr operator detail::int128() const noexcept;
    explicit constexpr operator detail::uint128() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator detail::int128_t() const noexcept;
    explicit constexpr operator detail::uint128_t() const noexcept;
    #endif

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

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
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

    template <typename Integer>
    friend constexpr auto operator+(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator-(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator-(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator*(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator*(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator/(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator/(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator%(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    // 3.2.4.5 Increment and Decrement
    constexpr auto operator++()    noexcept -> decimal128&;
    constexpr auto operator++(int) noexcept -> decimal128;  // NOLINT : C++14 so constexpr implies const
    constexpr auto operator--()    noexcept -> decimal128&;
    constexpr auto operator--(int) noexcept -> decimal128;  // NOLINT : C++14 so constexpr implies const

    // 3.2.4.6 Compound Assignment
    constexpr auto operator+=(decimal128 rhs) noexcept -> decimal128&;

    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&);

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&);

    constexpr auto operator-=(decimal128 rhs) noexcept -> decimal128&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&);

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&);

    constexpr auto operator*=(decimal128 rhs) noexcept -> decimal128&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&);

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&);

    constexpr auto operator/=(decimal128 rhs) noexcept -> decimal128&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&);

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&);

    constexpr auto operator%=(decimal128 rhs) noexcept -> decimal128&;

    // 3.2.9 Comparison operators:
    // Equality
    friend constexpr auto operator==(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Inequality
    friend constexpr auto operator!=(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less
    friend constexpr auto operator<(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less equal
    friend constexpr auto operator<=(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater
    friend constexpr auto operator>(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater equal
    friend constexpr auto operator>=(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // C++20 spaceship
    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal128 lhs, decimal128 rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    #if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
    friend inline std::string bit_string(decimal128 rhs) noexcept;
    #endif

    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd128(decimal128 lhs, decimal128 rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd128(decimal128 x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized128(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    // Bit-wise operators
    friend constexpr auto operator&(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator&(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator&(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator|(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator|(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator|(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator^(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator^(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator^(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator<<(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator<<(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator<<(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator>>(decimal128 lhs, decimal128 rhs) noexcept -> decimal128;

    template <typename Integer>
    friend constexpr auto operator>>(decimal128 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    template <typename Integer>
    friend constexpr auto operator>>(Integer lhs, decimal128 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128);

    friend constexpr auto operator~(decimal128 rhs) noexcept -> decimal128;

    // <cmath> functions that need to be friends
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend constexpr auto copysignd128(decimal128 mag, decimal128 sgn) noexcept -> decimal128;
    friend constexpr auto scalblnd128(decimal128 num, long exp) noexcept -> decimal128;
    friend constexpr auto scalbnd128(decimal128 num, int exp) noexcept -> decimal128;
    friend constexpr auto fmad128(decimal128 x, decimal128 y, decimal128 z) noexcept -> decimal128;
};

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
inline std::string bit_string(decimal128 rhs) noexcept
{
    std::stringstream ss;
    ss << std::hex << rhs.bits_.high << rhs.bits_.low;
    return ss.str();
}
#endif

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

    switch (exp_comb_bits)
    {
        case detail::d128_comb_11_mask.high:
            expval = (bits_.high & detail::d128_comb_11_mask.high) >> (high_word_significand_bits + 1);
            break;
        case detail::d128_comb_10_mask.high:
            expval = UINT64_C(0b10000000000000);
            break;
        case detail::d128_comb_01_mask.high:
            expval = UINT64_C(0b01000000000000);
            break;
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
        significand = (bits_.high & detail::d128_comb_11_significand_bits.high) == detail::d128_comb_11_significand_bits.high ?
            detail::uint128{0b10010000000000000000000000000000000000000000000000,0} :
            detail::uint128{0b10000000000000000000000000000000000000000000000000,0};
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

template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr auto decimal128::edit_exponent(T expval) noexcept -> void
{
    *this = decimal128(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal128::edit_sign(bool sign) noexcept -> void
{
    if (sign)
    {
        bits_.high |= detail::d128_sign_mask.high;
    }
    else
    {
        bits_.high &= ~detail::d128_sign_mask.high;
    }
}

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#endif

// TODO(mborland): Rather than doing bitwise operations on the whole uint128 we should
// be able to only operate on the affected word
//
// e.g. for sign bits_.high |= detail::d128_sign_mask.high
#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
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
    if (unsigned_coeff_digits > detail::precision_v<decimal128> + 1)
    {
        const auto digits_to_remove {unsigned_coeff_digits - (detail::precision_v<decimal128> + 1)};

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic push
        #  pragma GCC diagnostic ignored "-Wconversion"
        #endif

        unsigned_coeff /= detail::pow10(static_cast<Unsigned_Integer>(digits_to_remove));

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic pop
        #endif

        exp += digits_to_remove;
        unsigned_coeff_digits -= digits_to_remove;
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

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_REAL Float>
#else
template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
#endif
BOOST_DECIMAL_CXX20_CONSTEXPR decimal128::decimal128(Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        *this = from_bits(detail::d128_nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        *this = from_bits(detail::d128_inf_mask);
    }
    else
    #endif
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

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

template <typename Float>
BOOST_DECIMAL_CXX20_CONSTEXPR auto decimal128::operator=(const Float& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal128&)
{
    *this = decimal128{val};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
constexpr decimal128::decimal128(Integer val) noexcept // NOLINT : Incorrect parameter is never used
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal128{static_cast<ConversionType>(val), 0};
}

template <typename Integer>
constexpr auto decimal128::operator=(const Integer& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&)
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal128{static_cast<ConversionType>(val), 0};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
#else
template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
#endif
constexpr decimal128::decimal128(Decimal val) noexcept
{
    *this = to_decimal<decimal128>(val);
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T>
#else
template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
#endif
constexpr decimal128::decimal128(bool coeff, T exp, bool sign) noexcept
{
    *this = decimal128(static_cast<std::int32_t>(coeff), exp, sign);
}

constexpr decimal128::operator bool() const noexcept
{
    constexpr decimal128 zero {0, 0};
    return *this != zero;
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

constexpr decimal128::operator detail::int128() const noexcept
{
    return to_integral_128<decimal128, detail::int128>(*this);
}

constexpr decimal128::operator detail::uint128() const noexcept
{
    return to_integral_128<decimal128, detail::uint128>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal128::operator detail::int128_t() const noexcept
{
    return to_integral_128<decimal128, detail::int128_t>(*this);
}

constexpr decimal128::operator detail::uint128_t() const noexcept
{
    return to_integral_128<decimal128, detail::uint128_t>(*this);
}

#endif //BOOST_DECIMAL_HAS_INT128

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

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
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
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check for IEEE requirement that nan != nan
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return equal_parts_impl<decimal128>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                        rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator==(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
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
        return !rhs.isneg();
    }
    #else
    if (!lhs.isneg() && rhs.isneg())
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
    #endif

    return less_parts_impl<decimal128>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                       rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator<(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(rhs))
    {
        return false;
    }
    #endif

    return !less_impl(rhs, lhs) && lhs != rhs;
}

constexpr auto operator<=(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(rhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

constexpr auto operator>(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs))
    {
        return false;
    }
    #endif

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(rhs))
    {
        return false;
    }
    #endif

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
constexpr auto operator<=>(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering)
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
constexpr auto operator<=>(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering)
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

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
std::ostream& operator<<( std::ostream& os, boost::decimal::detail::uint128_t v )
{
    char buffer[ 64 ];

    os << mini_to_chars( buffer, v );
    return os;
}
#endif
#endif

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4127) // If constexpr macro only works for C++17 and above
#endif

constexpr auto d128_div_impl(decimal128 lhs, decimal128 rhs, decimal128& q, decimal128& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128 zero {0, 0};
    constexpr decimal128 nan {boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask)};
    constexpr decimal128 inf {boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask)};

    const bool sign {lhs.isneg() != rhs.isneg()};

    const auto lhs_fp {fpclassify(lhs)};
    const auto rhs_fp {fpclassify(rhs)};

    if (lhs_fp == FP_NAN || rhs_fp == FP_NAN)
    {
        q = nan;
        r = nan;
        return;
    }

    switch (lhs_fp)
    {
        case FP_INFINITE:
            q = sign ? -inf : inf;
            r = zero;
            return;
        case FP_ZERO:
            q = sign ? -zero : zero;
            r = sign ? -zero : zero;
            return;
        default:
            static_cast<void>(lhs);
    }

    switch (rhs_fp)
    {
        case FP_ZERO:
            q = inf;
            r = zero;
            return;
        case FP_INFINITE:
            q = sign ? -zero : zero;
            r = lhs;
            return;
        default:
            static_cast<void>(rhs);
    }
    #else
    static_cast<void>(r);
    #endif

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal128>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal128>(sig_rhs, exp_rhs);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    detail::decimal128_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    detail::decimal128_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};
    detail::decimal128_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    q = decimal128(q_components.sig, q_components.exp, q_components.sign);
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

constexpr auto d128_mod_impl(decimal128 lhs, decimal128 rhs, const decimal128& q, decimal128& r) noexcept -> void
{
    constexpr decimal128 zero {0, 0};

    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal128(q_trunc) * rhs);
}

constexpr auto operator+(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal128 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }
    #endif

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

    const auto result {detail::d128_add_impl<detail::decimal128_components>(
                                     lhs_sig, lhs_exp, lhs.isneg(),
                                     rhs_sig, rhs_exp, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator+(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }
    #endif

    bool lhs_bigger {lhs > rhs};
    if (lhs.isneg() && (rhs < 0))
    {
        lhs_bigger = !lhs_bigger;
    }
    bool abs_lhs_bigger {abs(lhs) > detail::make_positive_unsigned(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal128>(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal128_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {static_cast<detail::uint128>(detail::make_positive_unsigned(rhs))};
    std::int32_t exp_rhs {0};
    detail::normalize<decimal128>(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::make_positive_unsigned(sig_rhs);
    auto rhs_components {detail::decimal128_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    if (!lhs_bigger)
    {
        detail::swap(lhs_components, rhs_components);
        lhs_bigger = !lhs_bigger;
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal128_components result {};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Lhs sig: " << lhs_components.sig
              << "\nLhs exp: " << lhs_components.exp
              << "\nRhs sig: " << rhs_components.sig
              << "\nRhs exp: " << rhs_components.exp << std::endl;
    #endif

    if (!lhs_components.sign && rhs_components.sign)
    {
        result = detail::d128_sub_impl<detail::decimal128_components>(
                lhs_components.sig, lhs_components.exp, lhs_components.sign,
                rhs_components.sig, rhs_components.exp, rhs_components.sign,
                abs_lhs_bigger);
    }
    else
    {
        result = detail::d128_add_impl<detail::decimal128_components>(
                lhs_components.sig, lhs_components.exp, lhs_components.sign,
                rhs_components.sig, rhs_components.exp, rhs_components.sign);
    }

    return decimal128(result.sig, result.exp, result.sign);
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return rhs + lhs;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal128 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }
    #endif

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs + (-rhs);
    }

    const bool abs_lhs_bigger {abs(lhs) > abs(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal128>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal128>(sig_rhs, exp_rhs);

    const auto result {detail::d128_sub_impl<detail::decimal128_components>(
            sig_lhs, exp_lhs, lhs.isneg(),
            sig_rhs, exp_rhs, rhs.isneg(),
            abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isinf(lhs) || isnan(lhs))
    {
        return lhs;
    }
    #endif

    if (!lhs.isneg() && (rhs < 0))
    {
        return lhs + detail::make_positive_unsigned(rhs);
    }

    const bool abs_lhs_bigger {abs(lhs) > detail::make_positive_unsigned(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal128>(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal128_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {static_cast<detail::uint128>(detail::make_positive_unsigned(rhs))};
    std::int32_t exp_rhs {0};
    detail::normalize<decimal128>(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs {detail::make_positive_unsigned(sig_rhs)};
    auto rhs_components {detail::decimal128_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {detail::d128_sub_impl<detail::decimal128_components>(
            lhs_components.sig, lhs_components.exp, lhs_components.sign,
            rhs_components.sig, rhs_components.exp, rhs_components.sign,
            abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isinf(rhs) || isnan(rhs))
    {
        return rhs;
    }
    #endif

    if (lhs >= 0 && rhs.isneg())
    {
        return lhs + (-rhs);
    }

    const bool abs_lhs_bigger {detail::make_positive_unsigned(lhs) > abs(rhs)};

    auto sig_lhs {static_cast<detail::uint128>(detail::make_positive_unsigned(lhs))};
    std::int32_t exp_lhs {0};
    detail::normalize<decimal128>(sig_lhs, exp_lhs);
    auto unsigned_sig_lhs {detail::make_positive_unsigned(sig_lhs)};
    auto lhs_components {detail::decimal128_components{unsigned_sig_lhs, exp_lhs, (lhs < 0)}};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal128>(sig_rhs, exp_rhs);
    auto rhs_components {detail::decimal128_components{sig_rhs, exp_rhs, rhs.isneg()}};

    const auto result {detail::d128_sub_impl<detail::decimal128_components>(
            lhs_components.sig, lhs_components.exp, lhs_components.sign,
            rhs_components.sig, rhs_components.exp, rhs_components.sign,
            abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

constexpr auto operator*(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal128 zero {0, 0};

    const auto non_finite {detail::check_non_finite(lhs, rhs)};
    if (non_finite != zero)
    {
        return non_finite;
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto lhs_zeros {detail::remove_trailing_zeros(lhs_sig)};
    lhs_sig = lhs_zeros.trimmed_number;
    lhs_exp += static_cast<std::int32_t>(lhs_zeros.number_of_removed_zeros);

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    const auto rhs_zeros {detail::remove_trailing_zeros(rhs_sig)};
    rhs_sig = rhs_zeros.trimmed_number;
    rhs_exp += static_cast<std::int32_t>(rhs_zeros.number_of_removed_zeros);

    const auto result {detail::d128_mul_impl<detail::decimal128_components>(
            lhs_sig, lhs_exp, lhs.isneg(),
            rhs_sig, rhs_exp, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto lhs_zeros {detail::remove_trailing_zeros(lhs_sig)};
    lhs_sig = lhs_zeros.trimmed_number;
    lhs_exp += static_cast<std::int32_t>(lhs_zeros.number_of_removed_zeros);
    auto lhs_components {detail::decimal128_components{lhs_sig, lhs_exp, lhs.isneg()}};

    auto rhs_sig {static_cast<detail::uint128>(detail::make_positive_unsigned(rhs))};
    std::int32_t rhs_exp {0};
    const auto rhs_zeros {detail::remove_trailing_zeros(rhs_sig)};
    rhs_sig = rhs_zeros.trimmed_number;
    rhs_exp += static_cast<std::int32_t>(rhs_zeros.number_of_removed_zeros);
    auto unsigned_sig_rhs {detail::make_positive_unsigned(rhs_sig)};
    auto rhs_components {detail::decimal128_components{unsigned_sig_rhs, rhs_exp, (rhs < 0)}};

    const auto result {detail::d128_mul_impl<detail::decimal128_components>(
            lhs_components.sig, lhs_components.exp, lhs_components.sign,
            rhs_components.sig, rhs_components.exp, rhs_components.sign)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return rhs * lhs;
}

constexpr auto operator/(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    decimal128 q {};
    decimal128 r {};
    d128_div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128 zero {0, 0};
    constexpr decimal128 nan {boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask)};
    constexpr decimal128 inf {boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask)};

    const bool sign {lhs.isneg() != (rhs < 0)};

    const auto lhs_fp {fpclassify(lhs)};

    switch (lhs_fp)
    {
        case FP_NAN:
            return nan;
        case FP_INFINITE:
            return inf;
        case FP_ZERO:
            return sign ? -zero : zero;
        default:
            static_cast<void>(lhs);
    }

    if (rhs == 0)
    {
        return sign ? -inf : inf;
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal128>(lhs_sig, lhs_exp);

    detail::decimal128_components lhs_components {lhs_sig, lhs_exp, lhs.isneg()};

    auto rhs_sig {detail::make_positive_unsigned(rhs)};
    std::int32_t rhs_exp {};
    detail::decimal128_components rhs_components {rhs_sig, rhs_exp, rhs < 0};
    detail::decimal128_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal128(q_components.sig, q_components.exp, q_components.sign);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128 zero {0, 0};
    constexpr decimal128 inf {boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask)};
    constexpr decimal128 nan {boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask)};

    const bool sign {(lhs < 0) != rhs.isneg()};

    const auto rhs_fp {fpclassify(rhs)};

    if (rhs_fp == FP_NAN)
    {
        return nan;
    }

    switch (rhs_fp)
    {
        case FP_INFINITE:
            return sign ? -zero : zero;
        case FP_ZERO:
            return sign ? -inf : inf;
        default:
            static_cast<void>(lhs);
    }
    #endif

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal128>(rhs_sig, rhs_exp);

    detail::decimal128_components lhs_components {detail::make_positive_unsigned(lhs), 0, lhs < 0};
    detail::decimal128_components rhs_components {rhs_sig, rhs_exp, rhs.isneg()};
    detail::decimal128_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal128(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto operator%(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    decimal128 q {};
    decimal128 r {};
    d128_div_impl(lhs, rhs, q, r);
    d128_mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal128::operator++() noexcept -> decimal128&
{
    constexpr decimal128 one{1, 0};
    *this = *this + one;
    return *this;
}

constexpr auto decimal128::operator++(int) noexcept -> decimal128
{
    return ++(*this);
}

constexpr auto decimal128::operator--() noexcept -> decimal128&
{
    constexpr decimal128 one{1, 0};
    *this = *this - one;
    return *this;
}

constexpr auto decimal128::operator--(int) noexcept -> decimal128
{
    return --(*this);
}

constexpr auto decimal128::operator+=(decimal128 rhs) noexcept -> decimal128&
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128::operator+=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128::operator+=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&)
{
    *this = *this + rhs;
    return *this;
}

constexpr auto decimal128::operator-=(decimal128 rhs) noexcept -> decimal128&
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128::operator-=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128::operator-=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&)
{
    *this = *this - rhs;
    return *this;
}

constexpr auto decimal128::operator*=(decimal128 rhs) noexcept -> decimal128&
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128::operator*=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128::operator*=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&)
{
    *this = *this * rhs;
    return *this;
}

constexpr auto decimal128::operator/=(decimal128 rhs) noexcept -> decimal128&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128::operator/=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128&)
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128::operator/=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128&)
{
    *this = *this / rhs;
    return *this;
}

constexpr auto decimal128::operator%=(decimal128 rhs) noexcept -> decimal128&
{
    *this = *this % rhs;
    return *this;
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd128(decimal128 lhs, decimal128 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    const auto lhs_fp {fpclassify(lhs)};
    const auto rhs_fp {fpclassify(rhs)};

    if ((lhs_fp == FP_NAN && rhs_fp == FP_NAN) || (lhs_fp == FP_INFINITE && rhs_fp == FP_INFINITE))
    {
        return true;
    }
    if ((lhs_fp == FP_NAN || rhs_fp == FP_INFINITE) || (rhs_fp == FP_NAN || lhs_fp == FP_INFINITE))
    {
        return false;
    }
    #endif

    return lhs.unbiased_exponent() == rhs.unbiased_exponent();
}

// 3.6.5
// Effects: if x is finite, returns its quantum exponent.
// Otherwise, a domain error occurs and INT_MIN is returned.
constexpr auto quantexpd128(decimal128 x) noexcept -> int
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(x))
    {
        return INT_MIN;
    }
    #endif

    return static_cast<int>(x.unbiased_exponent());
}

// 3.6.6
// Returns: a number that is equal in value (except for any rounding) and sign to x,
// and which has an exponent set to be equal to the exponent of y.
// If the exponent is being increased, the value is correctly rounded according to the current rounding mode;
// if the result does not have the same value as x, the "inexact" floating-point exception is raised.
// If the exponent is being decreased and the significand of the result has more digits than the type would allow,
// the "invalid" floating-point exception is raised and the result is NaN.
// If one or both operands are NaN the result is NaN.
// Otherwise, if only one operand is infinity, the "invalid" floating-point exception is raised and the result is NaN.
// If both operands are infinity, the result is DEC_INFINITY, with the same sign as x, converted to the type of x.
// The quantize functions do not signal underflow.
constexpr auto quantized128(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Return the correct type of nan
    if (isnan(lhs))
    {
        return lhs;
    }
    else if (isnan(rhs))
    {
        return rhs;
    }

    // If one is infinity then return a signaling NAN
    if (isinf(lhs) != isinf(rhs))
    {
        return boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask);
    }
    else if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }
    #endif

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr auto operator&(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    return from_bits(lhs.bits_ & rhs.bits_);
}

template <typename Integer>
constexpr auto operator&(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(lhs.bits_ & static_cast<detail::uint128>(rhs));
}

template <typename Integer>
constexpr auto operator&(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(static_cast<detail::uint128>(lhs) & rhs.bits_);
}

constexpr auto operator|(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    return from_bits(lhs.bits_ | rhs.bits_);
}

template <typename Integer>
constexpr auto operator|(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(lhs.bits_ | static_cast<detail::uint128>(rhs));
}

template <typename Integer>
constexpr auto operator|(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(static_cast<detail::uint128>(lhs) | rhs.bits_);
}

constexpr auto operator^(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    return from_bits(lhs.bits_ ^ rhs.bits_);
}

template <typename Integer>
constexpr auto operator^(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(lhs.bits_ ^ static_cast<detail::uint128>(rhs));
}

template <typename Integer>
constexpr auto operator^(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(static_cast<detail::uint128>(lhs) ^ rhs.bits_);
}

constexpr auto operator<<(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    return from_bits(lhs.bits_ << static_cast<std::uint64_t>(rhs.bits_));
}

template <typename Integer>
constexpr auto operator<<(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(lhs.bits_ << static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator<<(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(static_cast<detail::uint128>(lhs) << static_cast<std::uint64_t>(rhs.bits_));
}

constexpr auto operator>>(decimal128 lhs, decimal128 rhs) noexcept -> decimal128
{
    return from_bits(lhs.bits_ >> static_cast<std::uint64_t>(rhs.bits_));
}

template <typename Integer>
constexpr auto operator>>(decimal128 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(lhs.bits_ >> static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator>>(Integer lhs, decimal128 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128)
{
    return from_bits(static_cast<detail::uint128>(lhs) >> static_cast<std::uint64_t>(rhs.bits_));
}

constexpr auto operator~(decimal128 lhs) noexcept -> decimal128
{
    return from_bits(~lhs.bits_);
}

constexpr auto copysignd128(decimal128 mag, decimal128 sgn) noexcept -> decimal128
{
    mag.edit_sign(sgn.isneg());
    return mag;
}

constexpr auto scalblnd128(decimal128 num, long exp) noexcept -> decimal128
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal128 zero {0, 0};

    if (num == zero || exp == 0 || isinf(num) || isnan(num))
    {
        return num;
    }
    #endif

    num.edit_exponent(num.biased_exponent() + exp);

    return num;
}

constexpr auto scalbnd128(decimal128 num, int expval) noexcept -> decimal128
{
    return scalblnd128(num, static_cast<long>(expval));
}

} //namespace decimal
} //namespace boost

namespace std {

BOOST_DECIMAL_EXPORT template<>
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
