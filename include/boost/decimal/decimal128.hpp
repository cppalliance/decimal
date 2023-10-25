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

public:
    // 3.2.4.1 construct/copy/destroy
    constexpr decimal128() noexcept = default;

    // 3.2.5 initialization from coefficient and exponent:
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1>, bool> = true>
    constexpr decimal128(T1 coeff, T2 exp, bool sign = false) noexcept;
};

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
            bits_ |= detail::d128_sign_mask;
            isneg = true;
        }
    }
    else
    {
        if (sign)
        {
            bits_ |= detail::d128_sign_mask;
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
                bits_ = detail::d128_comb_inf_mask;
            }
        }
        else
        {
            bits_ = detail::d128_comb_inf_mask;
        }
    }
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DECIMAL128_HPP
