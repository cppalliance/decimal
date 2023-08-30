// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/fast_float/compute_float32.hpp>
#include <boost/decimal/detail/fast_float/compute_float64.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <type_traits>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cmath>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <climits>
#include <cwchar>

namespace boost { namespace decimal {

namespace detail {

// See section 3.5.2
static constexpr std::uint32_t inf_mask =   0b0'11110'000000'0000000000'0000000000;
static constexpr std::uint32_t nan_mask =   0b0'11111'000000'0000000000'0000000000;
static constexpr std::uint32_t snan_mask =  0b0'11111'100000'0000000000'0000000000;
static constexpr std::uint32_t comb_inf_mask = 0b11110;
static constexpr std::uint32_t comb_nan_mask = 0b11111;
static constexpr std::uint32_t exp_snan_mask = 0b100000;

// Values from IEEE 754-2019 table 3.6
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto storage_width = 32;
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto precision = 7;
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto bias = 101;
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto emax = 96;
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto emin = -95;
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto etiny = -bias;
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto combination_field_width = 11;
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto trailing_significand_field_width = 20;

// Other useful values
static constexpr std::uint32_t max_significand = 9'999'999;
BOOST_DECIMAL_ATTRIBUTE_UNUSED constexpr auto max_string_length = 15;

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeee (0TTT)[tttttttttt][tttttttttt]
static constexpr std::uint32_t comb_01_mask = 0b01000;
static constexpr std::uint32_t comb_10_mask = 0b10000;

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
static constexpr std::uint32_t comb_11_mask = 0b11000;
static constexpr std::uint32_t comb_11_exp_bits = 0b00110;
static constexpr std::uint32_t comb_11_significand_bits = 0b00001;

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1101 T (01)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1110 T (10)eeeeee (100T)[tttttttttt][tttttttttt]
// static constexpr std::uint32_t comb_1100_mask = 0b11000;
static constexpr std::uint32_t comb_1101_mask = 0b11010;
static constexpr std::uint32_t comb_1110_mask = 0b11100;

// Powers of 2 used to determine the size of the significand
static constexpr std::uint32_t no_combination = 0b1111111111'1111111111;
static constexpr std::uint32_t big_combination = 0b0111'1111111111'1111111111;

// Exponent fields
static constexpr std::uint32_t max_exp_no_combination = 0b111111;
static constexpr std::uint32_t exp_combination_field_mask = max_exp_no_combination;
static constexpr std::uint32_t exp_one_combination = 0b1'111111;
static constexpr std::uint32_t max_biased_exp = 0b10'111111;
static constexpr std::uint32_t small_combination_field_mask = 0b0000'0000'0111'0000'0000'0000'0000'0000;
static constexpr std::uint32_t big_combination_field_mask = 0b0000'0000'0001'0000'0000'0000'0000'0000;

// Constexpr construction from an uint32_t without having to memcpy
static constexpr std::uint32_t construct_sign_mask = 0b1'00000'000000'0000000000'0000000000;
static constexpr std::uint32_t construct_combination_mask = 0b0'11111'000000'0000000000'0000000000;
static constexpr std::uint32_t construct_exp_mask = 0b0'00000'111111'0000000000'0000000000;
static constexpr std::uint32_t construct_significand_mask = no_combination;

} // Namespace detail

// Converts the significand to 9 digits to remove the effects of cohorts.
template <typename T>
constexpr void normalize(std::uint32_t& significand, T& exp) noexcept
{
    auto digits = detail::num_digits(significand);

    while (digits < detail::precision)
    {
        significand *= 10;
        --exp;
        ++digits;
    }
}

// ISO/IEC DTR 24733
// 3.2.2 class decimal32
class decimal32 final
{
private:

    // MSVC pragma that GCC and clang also support
    #pragma pack(push, 1)
    struct data_layout_
    {
        #ifdef BOOST_DECIMAL_ENDIAN_LITTLE_BYTE

        std::uint32_t significand : 20;
        std::uint32_t exponent : 6;
        std::uint32_t combination_field : 5;
        std::uint32_t sign : 1;

        #else

        std::uint32_t sign : 1;
        std::uint32_t combination_field : 5;
        std::uint32_t exponent : 6;
        std::uint32_t significand : 20;

        #endif
    };
    #pragma pack(pop)

    data_layout_ bits_{};

    // Returns the un-biased (quantum) exponent
    constexpr std::uint32_t full_exponent() const noexcept;

    // Returns the biased exponent
    constexpr std::int32_t biased_exponent() const noexcept;

    // Returns the significand complete with the bits implied from the combination field
    constexpr std::uint32_t full_significand() const noexcept;
    constexpr bool isneg() const noexcept;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename TargetType>
    constexpr TargetType to_integral() const noexcept;

    friend constexpr void div_mod_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept;

    template <typename T>
    BOOST_DECIMAL_CXX20_CONSTEXPR T floating_conversion_impl() const noexcept;

    // Debug bit pattern
    friend constexpr decimal32 from_bits(std::uint32_t bits) noexcept;
    friend std::uint32_t to_bits(decimal32 rhs) noexcept;
    friend void debug_pattern(decimal32 rhs) noexcept;
public:
    // 3.2.2.1 construct/copy/destroy:
    constexpr decimal32() noexcept = default;

    // 3.2.2.2 Conversion from floating-point type
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal32(Float val) noexcept;

    // 3.2.2.3 Conversion from integral type
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    explicit constexpr decimal32(Integer val) noexcept;

    // 3.2.2.4 Conversion to integral type
    explicit constexpr operator int() const noexcept;
    explicit constexpr operator unsigned() const noexcept;
    explicit constexpr operator long() const noexcept;
    explicit constexpr operator unsigned long() const noexcept;
    explicit constexpr operator long long() const noexcept;
    explicit constexpr operator unsigned long long() const noexcept;

    // 3.2.5 initialization from coefficient and exponent:
    template <typename T, typename T2, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr decimal32(T coeff, T2 exp, bool sign = false) noexcept;

    constexpr decimal32(const decimal32& val) noexcept = default;
    constexpr decimal32& operator=(const decimal32& val) noexcept = default;
    constexpr decimal32(decimal32&& val) noexcept = default;
    constexpr decimal32& operator=(decimal32&& val) noexcept = default;

    // 3.2.6 Conversion to floating-point type
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator float() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator double() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator long double() const noexcept;

    // cmath functions that are easier as friends
    friend constexpr bool signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;
    friend constexpr bool isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;
    friend constexpr bool isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;
    friend constexpr bool issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;
    friend constexpr bool isfinite BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;
    friend constexpr bool isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;
    friend constexpr int fpclassify BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;
    friend constexpr decimal32 abs BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept;

    // 3.2.7 unary arithmetic operators:
    friend constexpr decimal32 operator+(decimal32 rhs) noexcept;
    friend constexpr decimal32 operator-(decimal32 rhs) noexcept;

    // 3.2.8 binary arithmetic operators:
    friend constexpr decimal32 operator+(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr decimal32 operator-(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr decimal32 operator*(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr decimal32 operator/(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr decimal32 operator%(decimal32 lhs, decimal32 rhs) noexcept;

    // 3.2.2.5 Increment and Decrement
    constexpr decimal32& operator++() noexcept;
    constexpr decimal32 operator++(int) noexcept; // NOLINT : C++14 so constexpr implies const
    constexpr decimal32& operator--() noexcept;
    constexpr decimal32 operator--(int) noexcept; // NOLINT : C++14 so constexpr implies const

    // 3.2.2.6 Compound assignment
    constexpr decimal32& operator+=(decimal32 rhs) noexcept;
    constexpr decimal32& operator-=(decimal32 rhs) noexcept;
    constexpr decimal32& operator*=(decimal32 rhs) noexcept;
    constexpr decimal32& operator/=(decimal32 rhs) noexcept;
    constexpr decimal32& operator%=(decimal32 rhs) noexcept;

    // 3.2.9 comparison operators:
    friend constexpr bool operator==(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr bool operator!=(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr bool operator<(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr bool operator<=(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr bool operator>(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr bool operator>=(decimal32 lhs, decimal32 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr std::strong_ordering operator<=>(decimal32 lhs, decimal32 rhs) noexcept;
    #endif

    // 3.2.10 Formatted input:
    template <typename charT, typename traits>
    friend std::basic_istream<charT, traits>& operator>>(std::basic_istream<charT, traits>& is, decimal32& d);

    // 3.2.11 Formatted output:
    template <typename charT, typename traits>
    friend std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, const decimal32& d);

    // <cmath> extensions
    // 3.6.4 Same Quantum
    friend constexpr bool samequantumd32(decimal32 lhs, decimal32 rhs) noexcept;

    // 3.6.5 Quantum exponent
    friend constexpr int quantexpd32(decimal32 x) noexcept;

    // 3.6.6 Quantize
    friend constexpr decimal32 quantized32(decimal32 lhs, decimal32 rhs) noexcept;

    // 3.8.2 strtod
    friend constexpr decimal32 strtod32(const char* str, char** endptr) noexcept;

    // 3.9.2 wcstod
    friend constexpr decimal32 wcstod32(const wchar_t* str, wchar_t** endptr) noexcept;
};

template <typename T, typename T2, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr decimal32::decimal32(T coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = detail::make_unsigned_t<T>;

    static_assert(detail::is_integral_v<T>, "Coefficient must be an integer");
    static_assert(detail::is_integral_v<T2>, "Exponent must be an integer");

    Unsigned_Integer unsigned_coeff {};
    BOOST_DECIMAL_IF_CONSTEXPR (std::numeric_limits<T>::is_signed)
    {
        bits_.sign = coeff < 0 || sign;
        unsigned_coeff = coeff < 0 ? detail::apply_sign(coeff) : coeff;
    }
    else
    {
        bits_.sign = sign;
        unsigned_coeff = coeff;
    }

    // If the coeff is not in range make it so
    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision};
    while (unsigned_coeff_digits > detail::precision + 1)
    {
        unsigned_coeff /= 10;
        ++exp;
        --unsigned_coeff_digits;
    }

    if (reduced)
    {
        const auto trailing_num {unsigned_coeff % 10};
        unsigned_coeff /= 10;
        ++exp;
        if (trailing_num >= 5)
        {
            ++unsigned_coeff;
        }

        // If the significand was e.g. 99'999'999 rounding up
        // would put it out of range again
        if (unsigned_coeff > detail::max_significand)
        {
            unsigned_coeff /= 10;
            ++exp;
        }
    }

    auto reduced_coeff {static_cast<std::uint32_t>(unsigned_coeff)};

    // zero the combination field, so we can mask in the following
    bits_.combination_field = 0;
    bits_.significand = 0;
    bool big_combination {false};

    if (reduced_coeff == 0)
    {
        bits_.significand = 0;
        bits_.combination_field = 0;

        exp = 0;
    }
    else if (reduced_coeff < detail::no_combination)
    {
        // If the coefficient fits directly we don't need to use the combination field
        bits_.significand = reduced_coeff;
    }
    else if (reduced_coeff < detail::big_combination)
    {
        // Break the number into 3 bits for the combination field and 20 bits for the significand field

        // Use the least significant 20 bits to set the significand
        bits_.significand = reduced_coeff & detail::no_combination;

        // Now set the combination field (maximum of 3 bits)
        auto remaining_bits {reduced_coeff & detail::small_combination_field_mask};
        remaining_bits >>= 20;

        bits_.combination_field |= remaining_bits;
    }
    else
    {
        // Have to use the full combination field
        bits_.combination_field |= detail::comb_11_mask;
        big_combination = true;

        bits_.significand = reduced_coeff & detail::no_combination;
        const auto remaining_bit {reduced_coeff & detail::big_combination_field_mask};

        if (remaining_bit)
        {
            bits_.combination_field |= 1U;
        }
    }

    // If the exponent fits we do not need to use the combination field
    std::uint32_t biased_exp {static_cast<std::uint32_t>(exp + detail::bias)};
    const std::uint32_t biased_exp_low_six {biased_exp & detail::exp_combination_field_mask};
    if (biased_exp <= detail::max_exp_no_combination)
    {
        bits_.exponent = biased_exp;
    }
    else if (biased_exp <= detail::exp_one_combination)
    {
        if (big_combination)
        {
            bits_.combination_field |= detail::comb_1101_mask;
        }
        else
        {
            bits_.combination_field |= detail::comb_01_mask;
        }

        bits_.exponent = biased_exp_low_six;
    }
    else if (biased_exp <= detail::max_biased_exp)
    {
        if (big_combination)
        {
            bits_.combination_field |= detail::comb_1110_mask;
        }
        else
        {
            bits_.combination_field |= detail::comb_10_mask;
        }

        bits_.exponent = biased_exp_low_six;
    }
    else
    {
        // The value is probably infinity

        // If we can offset some extra power in the coefficient try to do so
        const auto coeff_dig {detail::num_digits(reduced_coeff)};
        if (coeff_dig < detail::precision)
        {
            for (auto i {coeff_dig}; i <= detail::precision; ++i)
            {
                reduced_coeff *= 10;
                --biased_exp;
                --exp;
                if (biased_exp == detail::max_biased_exp)
                {
                    break;
                }
            }

            if (detail::num_digits(reduced_coeff) <= detail::precision)
            {
                *this = decimal32(reduced_coeff, exp, static_cast<bool>(bits_.sign));
            }
            else
            {
                bits_.combination_field = detail::comb_inf_mask;
            }
        }
        else
        {
            bits_.combination_field = detail::comb_inf_mask;
        }
    }
}

constexpr decimal32 from_bits(std::uint32_t bits) noexcept
{
    decimal32 result;

    result.bits_.exponent = (bits & detail::construct_sign_mask) >> 31;
    result.bits_.combination_field = (bits & detail::construct_combination_mask) >> 26;
    result.bits_.exponent = (bits & detail::construct_exp_mask) >> 20;
    result.bits_.significand = bits & detail::construct_significand_mask;

    return result;
}

constexpr bool signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    return rhs.bits_.sign;
}

constexpr bool isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    return (rhs.bits_.combination_field & detail::comb_nan_mask) == detail::comb_nan_mask;
}

constexpr bool issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    return isnan(rhs) && (rhs.bits_.exponent & detail::exp_snan_mask) == detail::exp_snan_mask;
}

constexpr bool isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    return ((rhs.bits_.combination_field & detail::comb_inf_mask) == detail::comb_inf_mask) && !isnan(rhs);
}

constexpr bool isfinite BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    return !isinf(rhs) && !isnan(rhs);
}

constexpr bool isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.full_exponent()};

    if (exp <= detail::precision - 1)
    {
        return false;
    }

    return sig != 0 && isfinite(rhs);
}

constexpr int fpclassify BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    if (isinf(rhs))
    {
        return FP_INFINITE;
    }
    else if (isnan(rhs))
    {
        return FP_NAN;
    }
    else if (rhs.full_significand() == 0)
    {
        return FP_ZERO;
    }
    else if (!isnormal(rhs))
    {
        return FP_SUBNORMAL;
    }
    else
    {
        return FP_NORMAL;
    }
}

constexpr decimal32 abs BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept
{
    if (rhs.isneg())
    {
        return -rhs;
    }

    return rhs;
}

constexpr decimal32 operator+(decimal32 rhs) noexcept
{
    return rhs;
}

constexpr decimal32 operator-(decimal32 rhs) noexcept
{
    rhs.bits_.sign ^= 1;
    return rhs;
}

// Prioritizes checking for nans and then checks for infs
constexpr decimal32 check_non_finite(decimal32 lhs, decimal32 rhs) noexcept
{
    constexpr decimal32 zero {0, 0};

    if (isnan(lhs))
    {
        return lhs;
    }
    else if (isnan(rhs))
    {
        return rhs;
    }

    if (isinf(lhs))
    {
        return lhs;
    }
    else if (isinf(rhs))
    {
        return rhs;
    }

    return zero;
}

// We use kahan summation here where applicable
// https://en.wikipedia.org/wiki/Kahan_summation_algorithm
// NOLINTNEXTLINE : If addition is actually subtraction than change operator and vice versa
constexpr decimal32 operator+(decimal32 lhs, decimal32 rhs) noexcept
{
    constexpr decimal32 zero {0, 0};

    const auto res {check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }

    bool lhs_bigger {lhs > rhs};
    if (lhs.isneg() && rhs.isneg())
    {
        lhs_bigger = !lhs_bigger;
    }
    bool sign {};

    // Ensure that lhs is always the larger for ease of implementation
    if (!lhs_bigger)
    {
        detail::swap(lhs, rhs);
    }

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs - abs(rhs);
    }
    else if (lhs.isneg())
    {
        sign = true;
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.full_exponent()};
    normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.full_exponent()};
    normalize(sig_rhs, exp_rhs);

    auto delta_exp {exp_lhs > exp_rhs ? exp_lhs - exp_rhs : exp_rhs - exp_lhs};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "Starting sig lhs: " << sig_lhs
              << "\nStarting sig rhs: " << sig_rhs << std::endl;
    #endif

    if (delta_exp + 1 > detail::precision)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return lhs;
    }
    else if (delta_exp == detail::precision + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        if (sig_rhs >= UINT32_C(5'000'000))
        {
            ++sig_lhs;
            return {sig_lhs, static_cast<int>(exp_lhs) - detail::bias};
        }
        else
        {
            return lhs;
        }
    }

    bool carry {};

    // The two numbers can be added together without special handling

    // If we can add to the lhs sig rather than dividing we can save some precision
    // 32-bit signed int can have 9 digits and our normalized significand has 7
    if (delta_exp <= 2)
    {
        while (delta_exp > 0)
        {
            sig_lhs *= 10;
            --delta_exp;
            --exp_lhs;
        }
    }
    else
    {
        sig_lhs *= 100;
        delta_exp -= 2;
        exp_lhs -=2;
    }

    while (delta_exp > 1)
    {
        sig_rhs /= 10;
        --delta_exp;
    }


    if (delta_exp == 1)
    {
        auto carry_dig = sig_rhs % 10;
        sig_rhs /= 10;
        if (carry_dig >= 5)
        {
            carry = true;
        }
    }

    // Cast the results to signed types so that we can apply a sign at the end if necessary
    // Both of the significands are maximally 24 bits, so they fit into a 32-bit signed type just fine
    auto new_sig {static_cast<std::int32_t>(sig_lhs + sig_rhs) + static_cast<std::int32_t>(carry)};
    const auto new_exp {static_cast<int>(exp_lhs) - detail::bias};

    if (sign)
    {
        new_sig = -new_sig;
    }

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "Final sig lhs: " << sig_lhs
              << "\nFinal sig rhs: " << sig_rhs
              << "\nResult sig: " << new_sig << std::endl;
    #endif

    return {new_sig, new_exp};
}

constexpr decimal32& decimal32::operator++() noexcept
{
    constexpr decimal32 one(1, 0);
    *this = *this + one;
    return *this;
}

constexpr decimal32 decimal32::operator++(int) noexcept // NOLINT
{
    return ++(*this);
}

constexpr decimal32& decimal32::operator+=(decimal32 rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr decimal32 operator-(decimal32 lhs, decimal32 rhs) noexcept
{
    constexpr decimal32 zero {0, 0};

    const auto res {check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs + (-rhs);
    }

    const bool lhs_bigger {lhs > rhs};
    const bool abs_lhs_bigger {abs(lhs) > abs(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.full_exponent()};
    normalize(sig_lhs, exp_lhs);

    auto signed_sig_lhs = static_cast<std::int32_t>(sig_lhs);
    signed_sig_lhs = lhs.isneg() ? -signed_sig_lhs : signed_sig_lhs;

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.full_exponent()};
    normalize(sig_rhs, exp_rhs);

    auto signed_sig_rhs = static_cast<std::int32_t>(sig_rhs);
    signed_sig_rhs = rhs.isneg() ? -signed_sig_rhs : signed_sig_rhs;

    auto delta_exp {exp_lhs > exp_rhs ? exp_lhs - exp_rhs : exp_rhs - exp_lhs};

    if (delta_exp + 1 > detail::precision)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20

        return lhs_bigger ? lhs : -rhs;
    }
    else if (delta_exp == detail::precision + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 - 9.876543e-2 = 1.234566e5

        if (sig_rhs >= UINT32_C(5'000'000))
        {
            --sig_lhs;
            return {sig_lhs, static_cast<int>(exp_lhs) - detail::bias};
        }
        else
        {
            return lhs;
        }
    }

    // The two numbers can be subtracted together without special handling
    if (abs_lhs_bigger)
    {
        if (delta_exp <= 2)
        {
            while (delta_exp > 0)
            {
                signed_sig_lhs *= 10;
                --delta_exp;
                --exp_lhs;
            }
        }
        else
        {
            signed_sig_lhs *= 100;
            delta_exp -= 2;
            exp_lhs -= 2;
        }

        while (delta_exp > 1)
        {
            signed_sig_rhs /= 10;
            --delta_exp;
        }

        if (delta_exp == 1)
        {
            const auto carry_dig {signed_sig_rhs % 10};
            signed_sig_rhs /= 10;
            if (carry_dig >= 5)
            {
                ++signed_sig_rhs;
            }
            else if (carry_dig <= -5)
            {
                --signed_sig_rhs;
            }
        }
    }
    else
    {
        if (delta_exp <= 2)
        {
            while (delta_exp > 0)
            {
                signed_sig_rhs *= 10;
                --delta_exp;
                --exp_rhs;
            }
        }
        else
        {
            signed_sig_rhs *= 100;
            delta_exp -= 2;
            exp_rhs -= 2;
        }

        while (delta_exp > 1)
        {
            signed_sig_lhs /= 10;
            --delta_exp;
        }

        if (delta_exp == 1)
        {
            const auto carry_dig {signed_sig_lhs % 10};
            signed_sig_lhs /= 10;

            if (carry_dig >= 5)
            {
                ++signed_sig_lhs;
            }
            else if (carry_dig <= -5)
            {
                --signed_sig_rhs;
            }

        }
    }

    // Both of the significands are less than 9'999'999, so we can safely
    // cast them to signed 32-bit ints to calculate the new significand
    std::int32_t new_sig {}; // NOLINT : Value is never used but can't leave uninitialized in constexpr function

    if (rhs.isneg() && !lhs.isneg())
    {
        new_sig = signed_sig_lhs + signed_sig_rhs;
    }
    else
    {
        new_sig = signed_sig_lhs - signed_sig_rhs;
    }
    const auto new_exp {(abs_lhs_bigger ? static_cast<int>(exp_lhs) : static_cast<int>(exp_rhs)) - detail::bias};

    return {new_sig, new_exp};
}

constexpr decimal32& decimal32::operator--() noexcept
{
    constexpr decimal32 one(1, 0);
    *this = *this - one;
    return *this;
}

constexpr decimal32 decimal32::operator--(int) noexcept // NOLINT
{
    return --(*this);
}

constexpr decimal32& decimal32::operator-=(decimal32 rhs) noexcept
{
    *this = *this - rhs;
    return *this;
}

constexpr bool operator==(decimal32 lhs, decimal32 rhs) noexcept
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    if (lhs.bits_.sign != rhs.bits_.sign)
    {
        return false;
    }

    std::uint32_t lhs_real_exp {lhs.full_exponent()};
    std::uint32_t rhs_real_exp {rhs.full_exponent()};
    std::uint32_t lhs_significand {lhs.full_significand()};
    std::uint32_t rhs_significand {rhs.full_significand()};

    // Normalize the significands
    normalize(lhs_significand, lhs_real_exp);
    normalize(rhs_significand, rhs_real_exp);

    return lhs_real_exp == rhs_real_exp &&
           lhs_significand == rhs_significand;
}

constexpr bool operator!=(decimal32 lhs, decimal32 rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<(decimal32 lhs, decimal32 rhs) noexcept
{
    if (isnan(lhs) || isnan(rhs) ||
        (!lhs.bits_.sign && rhs.bits_.sign))
    {
        return false;
    }
    else if (lhs.bits_.sign && !rhs.bits_.sign)
    {
        return true;
    }
    else if (isfinite(lhs) && isinf(rhs))
    {
        if (!signbit(rhs))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    const bool both_neg {lhs.bits_.sign && rhs.bits_.sign};
    auto lhs_real_exp {lhs.biased_exponent()};
    auto rhs_real_exp {rhs.biased_exponent()};
    auto lhs_significand {lhs.full_significand()};
    auto rhs_significand {rhs.full_significand()};

    // Normalize the significands and exponents
    normalize(lhs_significand, lhs_real_exp);
    normalize(rhs_significand, rhs_real_exp);

    if (both_neg)
    {
        if (lhs_real_exp > rhs_real_exp)
        {
            return true;
        }
        else if (lhs_real_exp < rhs_real_exp)
        {
            return false;
        }
        else
        {
            return lhs_significand > rhs_significand;
        }
    }
    else
    {
        if (lhs_real_exp < rhs_real_exp)
        {
            return true;
        }
        else if (lhs_real_exp > rhs_real_exp)
        {
            return false;
        }
        else
        {
            return lhs_significand < rhs_significand;
        }
    }
}

constexpr bool operator<=(decimal32 lhs, decimal32 rhs) noexcept
{
    return !(rhs < lhs);
}

constexpr bool operator>(decimal32 lhs, decimal32 rhs) noexcept
{
    return rhs < lhs;
}

constexpr bool operator>=(decimal32 lhs, decimal32 rhs) noexcept
{
    return !(lhs < rhs);
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
constexpr std::strong_ordering operator<=>(decimal32 lhs, decimal32 rhs) noexcept
{
    if (lhs < rhs)
    {
        return std::strong_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::strong_ordering::greater;
    }

    return std::strong_ordering::equal;
}
#endif

constexpr std::uint32_t decimal32::full_exponent() const noexcept
{
    std::uint32_t exp {};

    if ((bits_.combination_field & detail::comb_11_mask) == 0b11000)
    {
        // bits 2 and 3 are the exp part of the combination field
        exp |= (bits_.combination_field & detail::comb_11_exp_bits) << 5;
    }
    else
    {
        // bits 0 and 1 are the exp part of the combination field
        exp |= (bits_.combination_field & detail::comb_11_mask) << 3;
    }

    exp |= bits_.exponent;

    return exp;
}

constexpr std::int32_t decimal32::biased_exponent() const noexcept
{
    return static_cast<std::int32_t>(full_exponent()) - detail::bias;
}

constexpr std::uint32_t decimal32::full_significand() const noexcept
{
    std::uint32_t significand {};

    if ((bits_.combination_field & detail::comb_11_mask) == 0b11000)
    {
        // Only need the one bit of T because the other 3 are implied
        if (bits_.combination_field & detail::comb_11_significand_bits)
        {
            significand = 0b1001'0000000000'0000000000;
        }
        else
        {
            significand = 0b1000'0000000000'0000000000;
        }
    }
    else
    {
        significand |= ((bits_.combination_field & 0b00111) << 20);
    }

    significand |= bits_.significand;

    return significand;
}

constexpr bool decimal32::isneg() const noexcept
{
    return static_cast<bool>(bits_.sign);
}

template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::decimal32(Float val) noexcept
{
    if (val != val)
    {
        *this = boost::decimal::from_bits(boost::decimal::detail::nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || -val == std::numeric_limits<Float>::infinity())
    {
        *this = boost::decimal::from_bits(boost::decimal::detail::inf_mask);
    }
    else
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};

        #ifdef BOOST_DECIMAL_DEBUG
        std::cerr << "Mant: " << components.mantissa
                  << "\nExp: " << components.exponent
                  << "\nSign: " << components.sign << std::endl;
        #endif

        if (components.exponent > detail::emax)
        {
            *this = boost::decimal::from_bits(boost::decimal::detail::inf_mask);
        }
        else
        {
            *this = decimal32 {components.mantissa, components.exponent, components.sign};
        }
    }
}

template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
constexpr decimal32::decimal32(Integer val) noexcept // NOLINT : Incorrect parameter is never used
{
    *this = decimal32{val, 0};
}

template <typename TargetType>
constexpr TargetType decimal32::to_integral() const noexcept
{
    TargetType result {};

    const bool this_is_neg {static_cast<bool>(this->bits_.sign)};
    const decimal32 unsigned_this {this_is_neg ? -*this : *this};
    constexpr decimal32 max_target_type {std::numeric_limits<TargetType>::max()};

    if (isnan(*this))
    {
        errno = EINVAL;
        return static_cast<TargetType>(0);
    }
    if (isinf(*this) || unsigned_this > max_target_type)
    {
        errno = ERANGE;
        return static_cast<TargetType>(0);
    }

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_unsigned<TargetType>::value)
    {
        if (this_is_neg)
        {
            errno = ERANGE;
            return static_cast<TargetType>(0);
        }
    }

    result = static_cast<TargetType>(this->full_significand());
    int exp {static_cast<int>(this->full_exponent()) - detail::bias};
    if (exp > 0)
    {
        result *= detail::pow10<TargetType>(exp);
    }
    else if (exp < 0)
    {
        result /= detail::pow10<TargetType>(-exp);
    }

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<TargetType>::value)
    {
        result = this_is_neg ? detail::apply_sign(result) : result;
    }

    return result;
}

constexpr decimal32::operator int() const noexcept
{
    return to_integral<int>();
}

constexpr decimal32::operator unsigned() const noexcept
{
    return to_integral<unsigned>();
}

constexpr decimal32::operator long() const noexcept
{
    return to_integral<long>();
}

constexpr decimal32::operator unsigned long() const noexcept
{
    return to_integral<unsigned long>();
}

constexpr decimal32::operator long long() const noexcept
{
    return to_integral<long long>();
}

constexpr decimal32::operator unsigned long long() const noexcept
{
    return to_integral<unsigned long long>();
}

template <typename charT, typename traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, const decimal32& d)
{
    if (issignaling(d))
    {
        if (d.isneg())
        {
            os << "-";
        }

        os << "nan(snan)";
        return os;
    }
    else if (isnan(d)) // only quiet NaNs left
    {
        if (d.isneg())
        {
            os << "-nan(ind)";
        }
        else
        {
            os << "nan";
        }

        return os;
    }
    else if (isinf(d))
    {
        if (d.isneg())
        {
            os << "-";
        }

        os << "inf";
        return os;
    }

    char buffer[detail::precision + 2] {}; // Precision + decimal point + null terminator

    if (d.bits_.sign == 1)
    {
        os << "-";
    }

    // Print the significand into the buffer so that we can insert the decimal point
    std::snprintf(buffer, sizeof(buffer), "%u", d.full_significand());
    std::memmove(buffer + 2, buffer + 1, detail::precision - 1);
    std::memset(buffer + 1, '.', 1);
    os << buffer;

    // Offset will adjust the exponent to compensate for adding the decimal point
    const auto offset {detail::num_digits(d.full_significand()) - 1};
    if (offset == 0)
    {
        os << "0";
    }

    os << "e";
    auto print_exp {static_cast<int>(d.full_exponent()) - detail::bias + offset};

    if (print_exp < 0)
    {
        os << "-";
        print_exp = -print_exp;
    }
    else
    {
        os << "+";
    }

    if (print_exp < 10)
    {
        os << "0";
    }

    os << print_exp;

    return os;
}

std::uint32_t to_bits(decimal32 rhs) noexcept
{
    std::uint32_t bits;
    std::memcpy(&bits, &rhs.bits_, sizeof(std::uint32_t));
    return bits;
}

constexpr decimal32 operator*(decimal32 lhs, decimal32 rhs) noexcept
{
    constexpr decimal32 zero {0, 0};

    const auto res {check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    normalize(sig_rhs, exp_rhs);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs;
    #endif

    const bool sign {!(lhs.isneg() == rhs.isneg())};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents
    //
    // We use a 64 bit resultant significand because the two 23-bit unsigned significands will always fit

    auto res_sig {static_cast<std::uint64_t>(sig_lhs) * static_cast<std::uint64_t>(sig_rhs)};
    auto res_exp {exp_lhs + exp_rhs};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > 9)
    {
        res_sig /= detail::powers_of_10[sig_dig - 9];
        res_exp += sig_dig - 9;
    }

    auto res_sig_32 {static_cast<std::int32_t>(res_sig)};
    res_sig_32 = sign ? -res_sig_32 : res_sig_32;

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    return decimal32 {res_sig_32, res_exp};
}

constexpr decimal32& decimal32::operator*=(decimal32 rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

constexpr void div_mod_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept
{
    // Check pre-conditions
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 nan {boost::decimal::from_bits(boost::decimal::detail::snan_mask)};
    constexpr decimal32 inf {boost::decimal::from_bits(boost::decimal::detail::inf_mask)};

    const bool sign {!(lhs.isneg() == rhs.isneg())};

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
            q = inf;
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
            q = nan;
            r = nan;
            return;
        case FP_INFINITE:
            q = sign ? -zero : zero;
            r = lhs;
            return;
        default:
            static_cast<void>(rhs);
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    normalize(sig_rhs, exp_rhs);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    const auto big_sig_lhs {static_cast<std::uint64_t>(sig_lhs) * detail::powers_of_10[detail::precision]};
    exp_lhs -= 7;

    auto res_sig {big_sig_lhs / static_cast<std::uint64_t>(sig_rhs)};
    auto res_exp {exp_lhs - exp_rhs};

    // Let the constructor handle shrinking it back down and rounding correctly
    q = decimal32{res_sig, res_exp};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    r = lhs - decimal32(q.full_significand() % detail::precision) * rhs;
}

constexpr decimal32 operator/(decimal32 lhs, decimal32 rhs) noexcept
{
    decimal32 q {};
    decimal32 r {};
    div_mod_impl(lhs, rhs, q, r);

    return q;
}

constexpr decimal32& decimal32::operator/=(decimal32 rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

constexpr decimal32 operator%(decimal32 lhs, decimal32 rhs) noexcept
{
    decimal32 q {};
    decimal32 r {};
    div_mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr decimal32& decimal32::operator%=(boost::decimal::decimal32 rhs) noexcept
{
    *this = *this % rhs;
    return *this;
}

void debug_pattern(decimal32 rhs) noexcept
{
    std::cerr << "Sig: " << rhs.full_significand()
              << "\nExp: " << rhs.biased_exponent()
              << "\nNeg: " << rhs.isneg() << std::endl;
}

template <typename T>
BOOST_DECIMAL_CXX20_CONSTEXPR T decimal32::floating_conversion_impl() const noexcept
{
    bool success {};

    auto fp_class = fpclassify(*this);

    switch (fp_class)
    {
        case FP_NAN:
            if (issignaling(*this))
            {
                return std::numeric_limits<T>::signaling_NaN();
            }
            return std::numeric_limits<T>::quiet_NaN();
        case FP_INFINITE:
            return std::numeric_limits<T>::infinity();
        case FP_ZERO:
            return 0;
        default:
            static_cast<void>(success);
    }

    // The casts to result are redundant but in pre C++17 modes MSVC warns about implicit conversions
    T result {};
    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<T, float>::value)
    {
        result = static_cast<T>(detail::fast_float::compute_float32(this->biased_exponent(), this->full_significand(), this->isneg(), success));
    }
    else BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<T, double>::value)
    {
        result = static_cast<T>(detail::fast_float::compute_float64(this->biased_exponent(), this->full_significand(), this->isneg(), success));
    }

    if (!success)
    {
        errno = EINVAL;
        return 0;
    }

    return result;
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::operator float() const noexcept
{
    return this->floating_conversion_impl<float>();
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::operator double() const noexcept
{
    return this->floating_conversion_impl<double>();
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::operator long double() const noexcept
{
    // Double already has more range and precision than a decimal32 will ever be able to provide
    return static_cast<long double>(this->floating_conversion_impl<double>());
}

template <typename charT, typename traits>
std::basic_istream<charT, traits>& operator>>(std::basic_istream<charT, traits>& is, decimal32& d)
{
    char buffer[1024] {}; // What should be an unreasonably high maximum
    is >> buffer;

    bool sign {};
    std::uint64_t significand {};
    std::int32_t exp {};
    const auto buffer_len {std::strlen(buffer)};

    if (buffer_len == 0)
    {
        errno = EINVAL;
        return is;
    }

    const auto r {detail::parser(buffer, buffer + buffer_len, sign, significand, exp)};

    if (r.ec != std::errc{})
    {
        if (r.ec == std::errc::not_supported)
        {
            if (significand)
            {
                d = from_bits(boost::decimal::detail::snan_mask);
            }
            else
            {
                d = from_bits(boost::decimal::detail::nan_mask);
            }
        }
        else if (r.ec == std::errc::value_too_large)
        {
            d = from_bits(boost::decimal::detail::inf_mask);
        }
        else
        {
            d = from_bits(boost::decimal::detail::snan_mask);
            errno = static_cast<int>(r.ec);
        }
    }
    else
    {
        d = decimal32(significand, exp, sign);
    }

    return is;
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr bool samequantumd32(decimal32 lhs, decimal32 rhs) noexcept
{
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

    return lhs.full_exponent() == rhs.full_exponent();
}

// 3.6.5
// Effects: if x is finite, returns its quantum exponent.
// Otherwise, a domain error occurs and INT_MIN is returned.
constexpr int quantexpd32(decimal32 x) noexcept
{
    if (!isfinite(x))
    {
        return INT_MIN;
    }

    return static_cast<int>(x.full_exponent());
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
constexpr decimal32 quantized32(decimal32 lhs, decimal32 rhs) noexcept
{
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
        return boost::decimal::from_bits(boost::decimal::detail::snan_mask);
    }
    else if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr decimal32 strtod32(const char* str, char** endptr) noexcept
{
    if (str == nullptr)
    {
        errno = EINVAL;
        return boost::decimal::from_bits(boost::decimal::detail::snan_mask);
    }

    bool sign {};
    std::uint64_t significand {};
    std::int32_t exp {};
    const auto buffer_len {detail::strlen(str)};

    if (buffer_len == 0)
    {
        errno = EINVAL;
        return from_bits(boost::decimal::detail::snan_mask);
    }

    const auto r {detail::parser(str, str + buffer_len, sign, significand, exp)};
    decimal32 d;

    if (r.ec != std::errc{})
    {
        if (r.ec == std::errc::not_supported)
        {
            if (significand)
            {
                d = from_bits(boost::decimal::detail::snan_mask);
            }
            else
            {
                d = from_bits(boost::decimal::detail::nan_mask);
            }
        }
        else if (r.ec == std::errc::value_too_large)
        {
            d = from_bits(boost::decimal::detail::inf_mask);
        }
        else
        {
            d = from_bits(boost::decimal::detail::snan_mask);
            errno = static_cast<int>(r.ec);
        }
    }
    else
    {
        d = decimal32(significand, exp, sign);
    }

    *endptr = const_cast<char*>(str + (r.ptr - str));
    return d;
}

constexpr decimal32 wcstod32(const wchar_t* str, wchar_t** endptr) noexcept
{
    char buffer[1024] {};
    if (str == nullptr || detail::strlen(str) > sizeof(buffer))
    {
        errno = EINVAL;
        return boost::decimal::from_bits(boost::decimal::detail::snan_mask);
    }

    // Convert all the characters from wchar_t to char and use regular strtod32
    for (std::size_t i {}; i < detail::strlen(str); ++i)
    {
        auto val {*(str + i)};
        if (BOOST_DECIMAL_UNLIKELY(val < 0 || val > 255))
        {
            // Character can not be converted
            break;
        }

        buffer[i] = static_cast<char>(val);
    }

    char* short_endptr {};
    const auto return_val {strtod32(buffer, &short_endptr)};

    *endptr = const_cast<wchar_t*>(str + (short_endptr - buffer));
    return return_val;
}

}} // Namespace boost::decimal

namespace std {

template <>
#ifdef BOOST_MSVC
class numeric_limits<boost::decimal::decimal32>
#else
struct numeric_limits<boost::decimal::decimal32>
#endif
{

#ifdef BOOST_MSVC
public:
#endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_specialized = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_signed = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_integer = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_exact = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_infinity = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_quiet_NaN = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_signaling_NaN = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_denorm_loss = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_round_style round_style = std::round_indeterminate;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_iec559 = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_bounded = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_modulo = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int digits = 7;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int radix = 10;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int min_exponent = -95;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int min_exponent10 = min_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_exponent = 96;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_exponent10 = max_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool traps = numeric_limits<std::uint32_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = true;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 (min)() { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 (max)() { return {9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 lowest() { return {-9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 epsilon() { return {1, -7}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 round_error() { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 infinity() { return boost::decimal::from_bits(boost::decimal::detail::inf_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 quiet_NaN() { return boost::decimal::from_bits(boost::decimal::detail::nan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 signaling_NaN() { return boost::decimal::from_bits(boost::decimal::detail::snan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 denorm_min() { return {1, boost::decimal::detail::etiny}; }
};

} // Namespace std

#endif // BOOST_DECIMAL_DECIMAL32_HPP
