// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL64_HPP
#define BOOST_DECIMAL_DECIMAL64_HPP

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
#include <boost/decimal/detail/fast_float/compute_float32.hpp>
#include <boost/decimal/detail/fast_float/compute_float64.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>

namespace boost {
namespace decimal {
namespace detail {

// See IEEE 754 dection 3.5.2
static constexpr auto d64_inf_mask = UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_nan_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_snan_mask = UINT64_C(0b0'11111'10000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_comb_inf_mask = UINT64_C(0b11110);
static constexpr auto d64_comb_nan_mask = UINT64_C(0b11111);
static constexpr auto d64_exp_snan_mask = UINT64_C(0b10000000);

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
static constexpr std::uint64_t d64_comb_01_mask = 0b01000;
static constexpr std::uint64_t d64_comb_10_mask = 0b10000;

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
static constexpr std::uint64_t d64_comb_11_mask = 0b11000;
static constexpr std::uint64_t d64_comb_11_exp_bits = 0b00110;
static constexpr std::uint64_t d64_comb_11_significand_bits = 0b00001;

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1101 T (01)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1110 T (10)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
static constexpr std::uint64_t d64_comb_1101_mask = 0b11010;
static constexpr std::uint64_t d64_comb_1110_mask = 0b11100;

// Powers of 2 used to determine the size of the significand
static constexpr std::uint64_t d64_no_combination = 0b1111111111'1111111111'1111111111'1111111111'1111111111;
static constexpr std::uint64_t d64_big_combination = 0b111'1111111111'1111111111'1111111111'1111111111'1111111111;

// Exponent fields
static constexpr std::uint64_t d64_max_exp_no_combination = 0b11111111;
static constexpr std::uint64_t d64_exp_combination_field_mask = d64_max_exp_no_combination;
static constexpr std::uint64_t d64_exp_one_combination = 0b1'11111111;
static constexpr std::uint64_t d64_max_biased_exp = 0b10'11111111;
static constexpr std::uint64_t d64_small_combination_field_mask = 0b111'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_big_combination_field_mask = 0b1'0000000000'0000000000'0000000000'0000000000'0000000000;

// Constexpr construction form an uint64_t without having to memcpy
static constexpr std::uint64_t d64_construct_sign_mask = 0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_combination_mask = 0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_exp_mask = 0b0'00000'11111111'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_significand_mask = d64_no_combination;

} //namespace detail

class decimal64 final
{
private:
    #pragma pack(push, 1)

    struct data_layout_
    {
        #ifdef BOOST_DECIMAL_ENDIAN_LITTLE_BYTE

        std::uint64_t significand : 50;
        std::uint64_t exponent : 8;
        std::uint64_t combination_field : 5;
        std::uint64_t sign : 1;

        #else

        std::uint64_t sign : 1;
        std::uint64_t combination_field : 5;
        std::uint64_t exponent : 8;
        std::uint64_t significand : 50;

        #endif
    };

    #pragma pack(pop)

    data_layout_ bits_ {};

    // Returns the un-biased (quantum) exponent
    constexpr auto unbiased_exponent() const noexcept -> std::uint64_t;

    // Returns the biased exponent
    constexpr auto biased_exponent() const noexcept -> std::int32_t;

    // Returns the significand complete with the bits implied from the combination field
    constexpr auto full_significand() const noexcept -> std::uint64_t;
    constexpr auto isneg() const noexcept -> bool;

    // Debug bit pattern
    friend constexpr auto from_bits(std::uint64_t bits) noexcept -> decimal64;
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(decimal64 rhs) noexcept -> std::uint64_t;

    // Equality template between any integer type and decimal32
    template <typename Decimal, typename Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    // Compares the components of the lhs with rhs for equality
    // Can be any type broken down into a sig and an exp that will be normalized for fair comparison
    template <typename T1, typename T2>
    friend constexpr auto equal_parts_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                           T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool;

    // Template to compare operator< for any integer type and decimal32
    template <typename Decimal, typename Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    // Implements less than using the components of lhs and rhs
    template <typename T1, typename T2>
    friend constexpr auto less_parts_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                          T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool;

public:
    // 3.2.3.1 construct/copy/destroy
    constexpr decimal64() noexcept = default;

    // 3.2.5 initialization from coefficient and exponent:
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1>, bool> = true>
    constexpr decimal64(T1 coeff, T2 exp, bool sign = false) noexcept;

    // cmath functions that are easier as friends
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;

    // 3.2.9 Comparison operators:
    // Equality
    friend constexpr auto operator==(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;
};

// 3.2.5 initialization from coefficient and exponent:
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1>, bool>>
constexpr decimal64::decimal64(T1 coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = detail::make_unsigned_t<T1>;

    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<T1>)
    {
        bits_.sign = coeff < 0 || sign;
    }
    else
    {
        bits_.sign = sign;
    }

    Unsigned_Integer unsigned_coeff = detail::make_positive_unsigned(coeff);

    // If the coeff is not in range make it so
    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision_v<decimal64>};
    while (unsigned_coeff_digits > detail::precision_v<decimal64> + 1)
    {
        unsigned_coeff /= 10;
        ++exp;
        --unsigned_coeff_digits;
    }

    // Round as required
    if (reduced)
    {
        exp += detail::fenv_round(unsigned_coeff, bits_.sign);
    }

    auto reduced_coeff {static_cast<std::uint64_t>(unsigned_coeff)};

    // zero the combination field, so we can mask in the following values
    bits_.combination_field = 0;
    bits_.significand = 0;
    bits_.exponent = 0;
    bool big_combination {false};

    if (reduced_coeff == 0)
    {
        bits_.significand = 0U;
        bits_.combination_field = 0U;

        exp = 0;
    }
    else if (reduced_coeff <= detail::d64_no_combination)
    {
        // If the coefficient fits directly we don't need to use the combination field
        bits_.significand = reduced_coeff;
    }
    else if (reduced_coeff <= detail::d64_big_combination)
    {
        // Break the number into 3 bits for the combination field and 50 bits for the significand field

        // Use the least significant 50 bits to set the significand
        bits_.significand = reduced_coeff & detail::d64_no_combination;

        // Now set the combination field (maximum of 3 bits)
        auto remaining_bits {reduced_coeff & detail::d64_small_combination_field_mask};
        remaining_bits >>= 50;

        bits_.combination_field |= remaining_bits;
    }
    else
    {
        // Have to use the full combination field
        bits_.combination_field |= detail::d64_comb_11_mask;
        big_combination = true;

        bits_.significand = reduced_coeff & detail::d64_no_combination;
        const auto remaining_bit {reduced_coeff & detail::d64_big_combination_field_mask};

        if (remaining_bit)
        {
            bits_.combination_field |= 1U;
        }
    }

    // If the exponent fits we do not need to use the combination field
    auto biased_exp {static_cast<std::uint64_t>(exp + detail::bias_v<decimal64>)};
    const auto biased_exp_low_eight {biased_exp & detail::d64_exp_combination_field_mask};

    if (biased_exp <= detail::d64_max_exp_no_combination)
    {
        bits_.exponent = biased_exp;
    }
    else if (biased_exp <= detail::d64_exp_one_combination)
    {
        if (big_combination)
        {
            bits_.combination_field |= detail::d64_comb_1101_mask;
        }
        else
        {
            bits_.combination_field |= detail::d64_comb_01_mask;
        }

        bits_.exponent = biased_exp_low_eight;
    }
    else if (biased_exp <= detail::d64_max_biased_exp)
    {
        if (big_combination)
        {
            bits_.combination_field |= detail::d64_comb_1110_mask;
        }
        else
        {
            bits_.combination_field |= detail::d64_comb_10_mask;
        }

        bits_.exponent = biased_exp_low_eight;
    }
    else
    {
        // The value is probably infinity

        // If we can offset some extra power in the coefficient try to do so
        const auto coeff_dig {detail::num_digits(reduced_coeff)};
        if (coeff_dig < detail::precision_v<decimal64>)
        {
            for (auto i {coeff_dig}; i <= detail::precision_v<decimal64>; ++i)
            {
                reduced_coeff *= 10;
                --biased_exp;
                --exp;
                if (biased_exp == detail::d64_max_biased_exp)
                {
                    break;
                }
            }

            if (detail::num_digits(reduced_coeff) <= detail::precision_v<decimal64>)
            {
                *this = decimal64(reduced_coeff, exp, static_cast<bool>(bits_.sign));
            }
            else
            {
                bits_.combination_field = detail::d64_comb_inf_mask;
            }
        }
        else
        {
            bits_.combination_field = detail::d64_comb_inf_mask;
        }
    }
}

constexpr auto decimal64::unbiased_exponent() const noexcept -> std::uint64_t
{
    std::uint64_t expval {};

    if ((bits_.combination_field & detail::d64_comb_11_mask) == detail::d64_comb_11_mask)
    {
        // bits 2 and 3 are the exp part of the combination field
        expval |= (bits_.combination_field & detail::d64_comb_11_exp_bits) << 5;
    }
    else
    {
        // bits 0 and 1 are the exp part of the combination field
        expval |= (bits_.combination_field & detail::d64_comb_11_mask) << 3;
    }

    expval |= bits_.exponent;

    return expval;
}

constexpr auto decimal64::biased_exponent() const noexcept -> std::int32_t
{
    return static_cast<std::int32_t>(unbiased_exponent()) - detail::bias_v<decimal64>;
}

constexpr auto decimal64::full_significand() const noexcept -> std::uint64_t
{
    std::uint64_t significand {};

    if ((bits_.combination_field & detail::d64_comb_11_mask) == detail::d64_comb_11_mask)
    {
        // Only need the one bit of T because the other 3 are implied
        if (bits_.combination_field & detail::d64_comb_11_significand_bits)
        {
            significand = 0b1001'0000000000'0000000000'0000000000'0000000000'0000000000;
        }
        else
        {
            significand = 0b1000'0000000000'0000000000'0000000000'0000000000'0000000000;
        }
    }
    else
    {
        significand |= ((bits_.combination_field & UINT64_C(0b00111)) << 50);
    }

    significand |= bits_.significand;

    return significand;
}

constexpr auto decimal64::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_.sign);
}

constexpr auto from_bits(std::uint64_t bits) noexcept -> decimal64
{
    decimal64 result;

    result.bits_.exponent          = (bits & detail::d64_construct_sign_mask) >> 63U;
    result.bits_.combination_field = (bits & detail::d64_construct_combination_mask) >> 58U;
    result.bits_.exponent          = (bits & detail::d64_construct_exp_mask) >> 50U;
    result.bits_.significand       =  bits & detail::d64_construct_significand_mask;

    return result;
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(decimal64 rhs) noexcept -> std::uint64_t
{
    const auto bits {detail::bit_cast<std::uint64_t>(rhs.bits_)};
    return bits;
}

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return static_cast<bool>(rhs.bits_.sign);
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return (rhs.bits_.combination_field & detail::d64_comb_nan_mask) == detail::d64_comb_nan_mask;
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return ((rhs.bits_.combination_field & detail::d64_comb_inf_mask) == detail::d64_comb_inf_mask) && (!isnan(rhs));
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return isnan(rhs) && (rhs.bits_.exponent & detail::d64_exp_snan_mask) == detail::d64_exp_snan_mask;
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision_v<decimal64> - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
}

constexpr auto operator==(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    // Check for IEEE requirement that nan != nan
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return equal_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                            rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator==(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(rhs, lhs);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DECIMAL64_HPP
