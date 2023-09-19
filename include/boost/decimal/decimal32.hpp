// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

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

#include <boost/decimal/fwd.hpp>
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

namespace boost { namespace decimal {

namespace detail {

// See section 3.5.2
static constexpr auto inf_mask      = UINT32_C(0b0'11110'000000'0000000000'0000000000);
static constexpr auto nan_mask      = UINT32_C(0b0'11111'000000'0000000000'0000000000);
static constexpr auto snan_mask     = UINT32_C(0b0'11111'100000'0000000000'0000000000);
static constexpr auto comb_inf_mask = UINT32_C(0b11110);
static constexpr auto comb_nan_mask = UINT32_C(0b11111);
static constexpr auto exp_snan_mask = UINT32_C(0b100000);

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

struct decimal32_components
{
    std::uint32_t sig;
    std::int32_t exp;
    bool sign;
};

template <typename Integer>
constexpr auto shrink_significand(Integer sig, std::int32_t& exp) noexcept -> std::uint32_t
{
    using Unsigned_Integer = detail::make_unsigned_t<Integer>;

    auto unsigned_sig {detail::make_positive_unsigned(sig)};
    const auto sig_dig {detail::num_digits(unsigned_sig)};

    if (sig_dig > 9)
    {
        unsigned_sig /= static_cast<Unsigned_Integer>(detail::powers_of_10[static_cast<std::size_t>(sig_dig - 9)]);
        exp += sig_dig - 9;
    }

    return static_cast<std::uint32_t>(unsigned_sig);
}

} // namespace detail

// Converts the significand to 7 digits to remove the effects of cohorts.
template <typename T, typename T2>
constexpr auto normalize(T& significand, T2& exp) noexcept -> void
{
    auto digits = detail::num_digits(significand);

    if (digits < detail::precision)
    {
        while (digits < detail::precision)
        {
            significand *= 10;
            --exp;
            ++digits;
        }
    }
    else if (digits > detail::precision)
    {
        while (digits > detail::precision + 1)
        {
            significand /= 10;
            ++exp;
            --digits;
        }

        exp += detail::fenv_round(significand, significand < 0);
    }
}

// ISO/IEC DTR 24733
// 3.2.2 class decimal32
class decimal32 final // NOLINT(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
{
private:

    using unsigned_layout_type = std::uint32_t;

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
    constexpr auto unbiased_exponent() const noexcept -> std::uint32_t;

    // Returns the biased exponent
    constexpr auto biased_exponent() const noexcept -> std::int32_t;

    // Returns the significand complete with the bits implied from the combination field
    constexpr auto full_significand() const noexcept -> std::uint32_t;
    constexpr auto isneg() const noexcept -> bool;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename TargetType>
    constexpr auto to_integral() const noexcept -> TargetType;

    friend constexpr auto generic_div_impl(detail::decimal32_components lhs, detail::decimal32_components rhs,
                                           detail::decimal32_components& q) noexcept -> void;
    friend constexpr auto div_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void;
    friend constexpr auto mod_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, int>;

    template<typename T>
    friend constexpr auto log10(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>;

    template <typename T>
    BOOST_DECIMAL_CXX20_CONSTEXPR auto floating_conversion_impl() const noexcept -> T;

    // Debug bit pattern
    friend constexpr auto from_bits(std::uint32_t bits) noexcept -> decimal32;
    friend auto to_bits(decimal32 rhs) noexcept -> std::uint32_t;
    friend auto debug_pattern(decimal32 rhs) noexcept -> void;

    // Equality template between any integer type and decimal32
    template <typename Integer>
    friend constexpr auto mixed_equality_impl(decimal32 lhs, Integer rhs) noexcept -> bool;

    // Compares the components of the lhs with rhs for equality
    // Can be any type broken down into a sig and an exp that will be normalized for fair comparison
    template <typename T, typename T2>
    friend constexpr auto equal_parts_impl(T lhs_sig, std::int32_t lhs_exp,
                                           T2 rhs_sig, std::int32_t rhs_exp) noexcept -> bool;

    // Template to compare operator< for any integer type and decimal32
    template <typename Integer>
    friend constexpr auto less_impl(decimal32 lhs, Integer rhs) noexcept -> bool;

    // Implements less than using the components of lhs and rhs
    template <typename T, typename T2>
    friend constexpr auto less_parts_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                          T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool;

    template <typename T, typename T2>
    friend constexpr auto add_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                   T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components;

    template <typename T, typename T2>
    friend constexpr auto sub_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                   T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign,
                                   bool lhs_bigger, bool abs_lhs_bigger) noexcept -> detail::decimal32_components;

    template <typename T, typename T2>
    friend constexpr auto mul_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                   T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components;

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
    constexpr decimal32(decimal32&& val) noexcept = default;
    constexpr auto operator=(const decimal32& val) noexcept -> decimal32& = default;
    constexpr auto operator=(decimal32&& val) noexcept -> decimal32& = default;

    // 3.2.6 Conversion to floating-point type
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator float() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator double() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator long double() const noexcept;

    // cmath functions that are easier as friends
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto isfinite    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto fpclassify  BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> int;
    friend constexpr auto abs         BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> decimal32;

    // 3.2.7 unary arithmetic operators:
    friend constexpr auto operator+(decimal32 rhs) noexcept -> decimal32;
    friend constexpr auto operator-(decimal32 rhs) noexcept -> decimal32;

    // 3.2.8 binary arithmetic operators:
    friend constexpr auto operator+(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator+(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator-(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator-(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator*(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator*(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator/(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator/(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator%(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    // 3.2.2.5 Increment and Decrement
    constexpr auto operator++()    noexcept -> decimal32&;
    constexpr auto operator++(int) noexcept -> decimal32;  // NOLINT : C++14 so constexpr implies const
    constexpr auto operator--()    noexcept -> decimal32&;
    constexpr auto operator--(int) noexcept -> decimal32;  // NOLINT : C++14 so constexpr implies const

    // 3.2.2.6 Compound assignment
    constexpr auto operator+=(decimal32 rhs) noexcept -> decimal32&;
    constexpr auto operator-=(decimal32 rhs) noexcept -> decimal32&;
    constexpr auto operator*=(decimal32 rhs) noexcept -> decimal32&;
    constexpr auto operator/=(decimal32 rhs) noexcept -> decimal32&;
    constexpr auto operator%=(decimal32 rhs) noexcept -> decimal32&;

    // 3.2.9 comparison operators:
    // Equality
    friend constexpr auto operator==(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Inequality
    friend constexpr auto operator!=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less
    friend constexpr auto operator<(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less equal
    friend constexpr auto operator<=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater
    friend constexpr auto operator>(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater equal
    friend constexpr auto operator>=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal32 lhs, decimal32 rhs) noexcept -> std::strong_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::strong_ordering>;

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::strong_ordering>;
    #endif

    // 3.2.10 Formatted input:
    template <typename charT, typename traits>
    friend auto operator>>(std::basic_istream<charT, traits>& is, decimal32& d) -> std::basic_istream<charT, traits>&;

    // 3.2.11 Formatted output:
    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, const decimal32& d) -> std::basic_ostream<charT, traits>&;

    // <cmath> extensions
    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd32(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd32(decimal32 x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized32(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    // 3.8.2 strtod
    friend constexpr auto strtod32(const char* str, char** endptr) noexcept -> decimal32;

    // 3.9.2 wcstod
    friend constexpr auto wcstod32(const wchar_t* str, wchar_t** endptr) noexcept-> decimal32;

    // <cmath> functions that need to be friends
    friend constexpr auto floord32(decimal32 val) noexcept -> decimal32;
    friend constexpr auto ceild32(decimal32 val) noexcept -> decimal32;
    friend constexpr auto fmodd32(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;
    friend constexpr auto copysignd32(decimal32 mag, decimal32 sgn) noexcept -> decimal32;
    friend constexpr auto modfd32(decimal32 x, decimal32* iptr) noexcept -> decimal32;

    // Related to <cmath>
    friend constexpr auto frexp10d32(decimal32 num, int* exp) noexcept -> std::int32_t;
    friend constexpr auto scalbnd32(decimal32 num, int exp) noexcept -> decimal32;
    friend constexpr auto scalblnd32(decimal32 num, long exp) noexcept -> decimal32;

    // These can be made public only for debugging matters
#ifndef BOOST_DECIMAL_DEBUG_MEMBERS
private:
#endif
    // Replaces the biased exponent with the value of exp
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr auto edit_exponent(T exp) noexcept -> void;

    // Replaces the value of the significand with sig
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr auto edit_significand(T sig) noexcept -> void;

    // Replaces the current sign with the one provided
    constexpr auto edit_sign(bool sign) noexcept -> void;
};

template <typename T, typename T2, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr decimal32::decimal32(T coeff, T2 exp, bool sign) noexcept // NOLINT(readability-function-cognitive-complexity,misc-no-recursion)
{
    using Unsigned_Integer = detail::make_unsigned_t<T>;

    static_assert(detail::is_integral_v<T>, "Coefficient must be an integer");
    static_assert(detail::is_integral_v<T2>, "Exponent must be an integer");

    Unsigned_Integer unsigned_coeff {};
    BOOST_DECIMAL_IF_CONSTEXPR (std::numeric_limits<T>::is_signed)
    {
        bits_.sign = coeff < 0 || sign;
        unsigned_coeff =
            static_cast<Unsigned_Integer>
            (
                coeff < static_cast<T>(0) ? static_cast<Unsigned_Integer>(detail::apply_sign(coeff)) : static_cast<Unsigned_Integer>(coeff)
            );
    }
    else
    {
        bits_.sign = sign;
        unsigned_coeff = static_cast<Unsigned_Integer>(coeff);
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

    // Round as required
    if (reduced)
    {
        exp += detail::fenv_round(unsigned_coeff, bits_.sign);
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
    else if (reduced_coeff <= detail::no_combination)
    {
        // If the coefficient fits directly we don't need to use the combination field
        bits_.significand = reduced_coeff;
    }
    else if (reduced_coeff <= detail::big_combination)
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

constexpr auto from_bits(std::uint32_t bits) noexcept -> decimal32
{
    decimal32 result;

    result.bits_.exponent          = static_cast<std::uint32_t>((bits & detail::construct_sign_mask)        >> 31U);
    result.bits_.combination_field = static_cast<std::uint32_t>((bits & detail::construct_combination_mask) >> 26U);
    result.bits_.exponent          = static_cast<std::uint32_t>((bits & detail::construct_exp_mask)         >> 20U);
    result.bits_.significand       = static_cast<std::uint32_t>( bits & detail::construct_significand_mask);

    return result;
}

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return rhs.bits_.sign;
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return (rhs.bits_.combination_field & detail::comb_nan_mask) == detail::comb_nan_mask;
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return isnan(rhs) && (rhs.bits_.exponent & detail::exp_snan_mask) == detail::exp_snan_mask;
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return ((rhs.bits_.combination_field & detail::comb_inf_mask) == detail::comb_inf_mask) && (!isnan(rhs));
}

constexpr auto isfinite BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return (!isinf(rhs)) && (!isnan(rhs));
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
}

constexpr auto fpclassify BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> int
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

constexpr auto abs BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> decimal32
{
    return (rhs.isneg()) ? -rhs : rhs;
}

constexpr auto operator+(decimal32 rhs) noexcept -> decimal32
{
    return rhs;
}

constexpr auto operator-(decimal32 rhs) noexcept-> decimal32
{
    rhs.bits_.sign ^= 1U;
    return rhs;
}

// Prioritizes checking for nans and then checks for infs
constexpr auto check_non_finite(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
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

template <typename T, typename T2>
constexpr auto add_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components
{
    const bool sign {lhs_sign};

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

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

        return {lhs_sig, lhs_exp, lhs_sign};
    }
    else if (delta_exp == detail::precision + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        if (rhs_sig >= UINT32_C(5'000'000))
        {
            ++lhs_sig;
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
    // 32-bit signed int can have 9 digits and our normalized significand has 7
    if (delta_exp <= 2)
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
        lhs_sig *= 100;
        delta_exp -= 2;
        lhs_exp -=2;
    }

    while (delta_exp > 1)
    {
        rhs_sig /= 10;
        --delta_exp;
    }


    if (delta_exp == 1)
    {
        detail::fenv_round(rhs_sig, rhs_sign);
    }

    // Cast the results to signed types so that we can apply a sign at the end if necessary
    // Both of the significands are maximally 24 bits, so they fit into a 32-bit signed type just fine
    const auto new_sig {static_cast<std::int32_t>(lhs_sig + rhs_sig)};
    const auto new_exp {lhs_exp};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "Final sig lhs: " << sig_lhs
              << "\nFinal sig rhs: " << sig_rhs
              << "\nResult sig: " << new_sig << std::endl;
    #endif

    return {res_sig, new_exp, sign};
}

template <typename T, typename T2>
constexpr auto sub_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign,
                        bool lhs_bigger, bool abs_lhs_bigger) noexcept -> detail::decimal32_components
{
    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};
    auto signed_sig_lhs {detail::make_signed_value(lhs_sig, lhs_sign)};
    auto signed_sig_rhs {detail::make_signed_value(rhs_sig, rhs_sign)};

    if (delta_exp + 1 > detail::precision)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20

        return lhs_bigger ? detail::decimal32_components{detail::shrink_significand(lhs_sig, lhs_exp), lhs_exp, lhs_sign} :
               detail::decimal32_components{detail::shrink_significand(rhs_sig, rhs_exp), rhs_exp, !rhs_sign};
    }
    else if (delta_exp == detail::precision + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 - 9.876543e-2 = 1.234566e5

        if (rhs_sig >= UINT32_C(5'000'000))
        {
            --lhs_sig;
            return {lhs_sig, lhs_exp, lhs_sign};
        }
        else
        {
            return {lhs_sig, lhs_exp, lhs_sign};
        }
    }

    // The two numbers can be subtracted together without special handling

    auto& sig_bigger {abs_lhs_bigger ? signed_sig_lhs : signed_sig_rhs};
    auto& exp_bigger {abs_lhs_bigger ? lhs_exp : rhs_exp};
    auto& sig_smaller {abs_lhs_bigger ? signed_sig_rhs : signed_sig_lhs};
    auto& smaller_sign {abs_lhs_bigger ? rhs_sign : lhs_sign};

    if (delta_exp == 1)
    {
        sig_bigger *= 10;
        --delta_exp;
        --exp_bigger;
    }
    else if (delta_exp >= 2)
    {
        sig_bigger *= 100;
        delta_exp -= 2;
        exp_bigger -= 2;
    }

    while (delta_exp > 1)
    {
        sig_smaller /= 10;
        --delta_exp;
    }

    if (delta_exp == 1)
    {
        detail::fenv_round(sig_smaller, smaller_sign);
    }

    // Both of the significands are less than 9'999'999, so we can safely
    // cast them to signed 32-bit ints to calculate the new significand
    std::int32_t new_sig {}; // NOLINT : Value is never used but can't leave uninitialized in constexpr function

    if (rhs_sign && !lhs_sign)
    {
        new_sig = signed_sig_lhs + signed_sig_rhs;
    }
    else
    {
        new_sig = signed_sig_lhs - signed_sig_rhs;
    }

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {res_sig, new_exp, new_sign};
}

// We use kahan summation here where applicable
// https://en.wikipedia.org/wiki/Kahan_summation_algorithm
// NOLINTNEXTLINE : If addition is actually subtraction than change operator and vice versa
constexpr auto operator+(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
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

    // Ensure that lhs is always the larger for ease of implementation
    if (!lhs_bigger)
    {
        detail::swap(lhs, rhs);
    }

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs - abs(rhs);
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    normalize(sig_rhs, exp_rhs);

    const auto result {add_impl(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator+(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }

    bool lhs_bigger {lhs > rhs};
    if (lhs.isneg() && (rhs < 0))
    {
        lhs_bigger = !lhs_bigger;
    }
    bool abs_lhs_bigger {abs(lhs) > detail::make_positive_unsigned(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);

    auto lhs_components {detail::decimal32_components{sig_lhs, exp_lhs, lhs.isneg()}};
    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal32_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    if (!lhs_bigger)
    {
        detail::swap(lhs_components, rhs_components);
        lhs_bigger = !lhs_bigger;
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal32_components result {};

    if (!lhs_components.sign && rhs_components.sign)
    {
        result = sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                          rhs_components.sig, rhs_components.exp, rhs_components.sign,
                          lhs_bigger, abs_lhs_bigger);
    }
    else
    {
        result = add_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                          rhs_components.sig, rhs_components.exp, rhs_components.sign);
    }

    return decimal32(result.sig, result.exp, result.sign);
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    return rhs + lhs;
}

constexpr auto decimal32::operator++() noexcept -> decimal32&
{
    constexpr decimal32 one(1, 0);
    *this = *this + one;
    return *this;
}

constexpr auto decimal32::operator++(int) noexcept -> decimal32
{
    return ++(*this);
}

constexpr auto decimal32::operator+=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this + rhs;
    return *this;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
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
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    normalize(sig_rhs, exp_rhs);

    const auto result {sub_impl(sig_lhs, exp_lhs, lhs.isneg(),
                                sig_rhs, exp_rhs, rhs.isneg(),
                                lhs_bigger, abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    if (isinf(lhs) || isnan(lhs))
    {
        return lhs;
    }

    if (!lhs.isneg() && (rhs < 0))
    {
        return lhs + (-rhs);
    }

    const bool lhs_bigger {lhs > rhs};
    const bool abs_lhs_bigger {abs(lhs) > detail::make_positive_unsigned(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal32_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal32_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                lhs_bigger, abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    if (isinf(lhs) || isnan(rhs))
    {
        return rhs;
    }

    if (!(lhs < 0) && rhs.isneg())
    {
        return lhs + (-rhs);
    }

    const bool lhs_bigger {lhs > rhs};
    const bool abs_lhs_bigger {detail::make_positive_unsigned(lhs) > rhs};

    auto sig_lhs {lhs};
    std::int32_t exp_lhs {0};
    normalize(sig_lhs, exp_lhs);
    auto unsigned_sig_lhs = detail::shrink_significand(detail::make_positive_unsigned(sig_lhs), exp_lhs);
    auto lhs_components {detail::decimal32_components{unsigned_sig_lhs, exp_lhs, (rhs < 0)}};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    normalize(sig_rhs, exp_rhs);
    auto rhs_components {detail::decimal32_components{sig_rhs, exp_rhs, rhs.isneg()}};

    const auto result {sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                lhs_bigger, abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

constexpr auto decimal32::operator--() noexcept -> decimal32&
{
    constexpr decimal32 one(1, 0);
    *this = *this - one;
    return *this;
}

constexpr auto decimal32::operator--(int) noexcept -> decimal32
{
    return --(*this);
}

constexpr auto decimal32::operator-=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this - rhs;
    return *this;
}

template <typename T, typename T2>
constexpr auto equal_parts_impl(T lhs_sig, std::int32_t lhs_exp, T2 rhs_sig, std::int32_t rhs_exp) noexcept -> bool
{
    normalize(lhs_sig, lhs_exp);
    normalize(rhs_sig, rhs_exp);

    return lhs_exp == rhs_exp && lhs_sig == rhs_sig;
}

constexpr auto operator==(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    if (lhs.bits_.sign != rhs.bits_.sign)
    {
        return false;
    }

    return equal_parts_impl(lhs.full_significand(), lhs.biased_exponent(),
                            rhs.full_significand(), rhs.biased_exponent());
}

template <typename Integer>
constexpr auto mixed_equality_impl(decimal32 lhs, Integer rhs) noexcept -> bool
{
    using Unsigned_Integer = detail::make_unsigned_t<Integer>;

    if (isnan(lhs) || isinf(lhs))
    {
        return false;
    }

    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<Integer>)
    {
        if (lhs.bits_.sign != static_cast<std::uint32_t>(rhs < 0))
        {
            return false;
        }
    }
    else
    {
        if (lhs.bits_.sign)
        {
            return false;
        }
    }

    const auto rhs_significand =
        static_cast<Unsigned_Integer>
        (
            (rhs < 0) ? static_cast<Unsigned_Integer>(detail::apply_sign(rhs)) : static_cast<Unsigned_Integer>(rhs)
        );

    return equal_parts_impl(lhs.full_significand(), lhs.biased_exponent(),
                            rhs_significand, INT32_C(0));
}

template <typename Integer>
constexpr auto operator==(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

template <typename T, typename T2>
constexpr auto less_parts_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                               T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool
{
    const bool both_neg {lhs_sign && rhs_sign};

    // Normalize the significands and exponents
    normalize(lhs_sig, lhs_exp);
    normalize(rhs_sig, rhs_exp);

    if (lhs_sig == 0 && rhs_sig != 0)
    {
        return (!rhs_sign);
    }
    else if (lhs_sig != 0 && rhs_sig == 0)
    {
        return lhs_sign;
    }
    else if (lhs_sig == 0 && rhs_sig == 0)
    {
        return false;
    }
    else if (both_neg)
    {
        if (lhs_exp > rhs_exp)
        {
            return true;
        }
        else if (lhs_exp < rhs_exp)
        {
            return false;
        }
        else
        {
            return (lhs_sig > rhs_sig);
        }
    }
    else
    {
        if ((lhs_exp < rhs_exp) && (lhs_sig != static_cast<T>(0)))
        {
            return true;
        }
        else if (lhs_exp > rhs_exp)
        {
            return false;
        }
        else
        {
            return (lhs_sig < rhs_sig);
        }
    }
}

constexpr auto operator<(decimal32 lhs, decimal32 rhs) noexcept -> bool
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

    return less_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                           rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto less_impl(decimal32 lhs, Integer rhs) noexcept -> bool
{
    using Unsigned_Integer = detail::make_unsigned_t<Integer>;

    if (isnan(lhs))
    {
        return false;
    }
    else if (isinf(lhs))
    {
        return lhs.isneg();
    }

    bool lhs_sign {lhs.isneg()};
    bool rhs_sign {false};

    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<Integer>)
    {
        if (rhs < 0)
        {
            rhs_sign = true;
        }

        if (lhs_sign && !rhs_sign)
        {
            return true;
        }
        else if (!lhs_sign && rhs_sign)
        {
            return false;
        }
    }
    else
    {
        if (lhs_sign)
        {
            return true;
        }
    }

    Unsigned_Integer rhs_significand {rhs_sign ? detail::apply_sign(rhs) : rhs};

    return less_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs_sign,
                           rhs_significand, INT32_C(0), rhs_sign);
}

template <typename Integer>
constexpr auto operator<(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !less_impl(rhs, lhs) && lhs != rhs;
}

constexpr auto operator<=(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(rhs < lhs);
}

constexpr auto operator>(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs < rhs);
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

constexpr auto operator<=>(decimal32 lhs, decimal32 rhs) noexcept -> std::strong_ordering
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

template <typename Integer>
constexpr auto operator<=>(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::strong_ordering>
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

template <typename Integer>
constexpr auto operator<=>(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::strong_ordering>
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

constexpr auto decimal32::unbiased_exponent() const noexcept -> std::uint32_t
{
    std::uint32_t expval {};

    if ((bits_.combination_field & detail::comb_11_mask) == 0b11000)
    {
        // bits 2 and 3 are the exp part of the combination field
        expval |= (bits_.combination_field & detail::comb_11_exp_bits) << 5;
    }
    else
    {
        // bits 0 and 1 are the exp part of the combination field
        expval |= (bits_.combination_field & detail::comb_11_mask) << 3;
    }

    expval |= bits_.exponent;

    return expval;
}

constexpr auto decimal32::biased_exponent() const noexcept -> std::int32_t
{
    return static_cast<std::int32_t>(unbiased_exponent()) - detail::bias;
}

template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr auto decimal32::edit_exponent(T expval) noexcept -> void
{
    *this = decimal32(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal32::full_significand() const noexcept -> std::uint32_t
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

template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr auto decimal32::edit_significand(T sig) noexcept -> void
{
    *this = decimal32(sig, this->biased_exponent(), this->isneg());
}

constexpr auto decimal32::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_.sign);
}

// Allows changing the sign even on nans and infs
constexpr auto decimal32::edit_sign(bool sign) noexcept -> void
{
    this->bits_.sign = static_cast<std::uint32_t>(sign);
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
constexpr auto decimal32::to_integral() const noexcept -> TargetType
{
    TargetType result {};

    const auto this_is_neg   = static_cast<bool>(this->bits_.sign);
    const auto unsigned_this = decimal32 {this_is_neg ? -*this : *this};

    constexpr auto max_target_type = decimal32 { (std::numeric_limits<TargetType>::max)() };

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
    int expval {static_cast<int>(this->unbiased_exponent()) - detail::bias};
    if (expval > 0)
    {
        result *= detail::pow10<TargetType>(expval);
    }
    else if (expval < 0)
    {
        result /= detail::pow10<TargetType>(-expval);
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
auto operator<<(std::basic_ostream<charT, traits>& os, const decimal32& d) -> std::basic_ostream<charT, traits>&
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
    auto print_exp {static_cast<int>(d.unbiased_exponent()) - detail::bias + offset};

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

auto to_bits(decimal32 rhs) noexcept -> std::uint32_t
{
    std::uint32_t bits;
    std::memcpy(&bits, &rhs.bits_, sizeof(std::uint32_t));
    return bits;
}

template <typename T, typename T2>
constexpr auto mul_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components
{
    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs;
    #endif

    bool sign {!(lhs_sign == rhs_sign)};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents
    //
    // We use a 64 bit resultant significand because the two 23-bit unsigned significands will always fit

    auto res_sig {static_cast<std::uint64_t>(lhs_sig) * static_cast<std::uint64_t>(rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > 9)
    {
        res_sig /= detail::powers_of_10[sig_dig - 9];
        res_exp += sig_dig - 9;
    }

    const auto res_sig_32 {static_cast<std::uint32_t>(res_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    if (res_sig_32 == 0)
    {
        sign = false;
    }

    return {res_sig_32, res_exp, sign};
}

constexpr auto operator*(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
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

    const auto result {mul_impl(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal32_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs {detail::shrink_significand(detail::make_positive_unsigned(sig_rhs), exp_rhs)};
    auto rhs_components {detail::decimal32_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {mul_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                rhs_components.sig, rhs_components.exp, rhs_components.sign)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    return rhs * lhs;
}

constexpr auto decimal32::operator*=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this * rhs;
    return *this;
}

constexpr auto generic_div_impl(detail::decimal32_components lhs, detail::decimal32_components rhs,
                                detail::decimal32_components& q) noexcept -> void
{
    bool sign {lhs.sign != rhs.sign};

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    const auto big_sig_lhs {static_cast<std::uint64_t>(lhs.sig) * detail::powers_of_10[detail::precision]};
    lhs.exp -= 7;

    auto res_sig {big_sig_lhs / static_cast<std::uint64_t>(rhs.sig)};
    auto res_exp {lhs.exp - rhs.exp};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > 9)
    {
        res_sig /= detail::powers_of_10[sig_dig - 9];
        res_exp += sig_dig - 9;
    }

    const auto res_sig_32 {static_cast<std::uint32_t>(res_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    if (res_sig_32 == 0)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    q = detail::decimal32_components{res_sig_32, res_exp, sign};
}

constexpr auto div_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void
{
    // Check pre-conditions
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 nan {boost::decimal::from_bits(boost::decimal::detail::snan_mask)};
    constexpr decimal32 inf {boost::decimal::from_bits(boost::decimal::detail::inf_mask)};

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

    detail::decimal32_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    detail::decimal32_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};
    detail::decimal32_components q_components {};

    generic_div_impl(lhs_components, rhs_components, q_components);

    q = decimal32(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto mod_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void
{
    constexpr decimal32 zero {0, 0};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    auto q_trunc {q > zero ? floord32(q) : ceild32(q)};
    r = lhs - (decimal32(q_trunc) * rhs);
}

constexpr auto operator/(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    decimal32 q {};
    decimal32 r {};
    div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    // Check pre-conditions
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 nan {boost::decimal::from_bits(boost::decimal::detail::snan_mask)};
    constexpr decimal32 inf {boost::decimal::from_bits(boost::decimal::detail::inf_mask)};

    const bool sign {lhs.isneg() != (rhs < 0)};

    const auto lhs_fp {fpclassify(lhs)};

    if (lhs_fp == FP_NAN)
    {
        return nan;
    }

    switch (lhs_fp)
    {
        case FP_INFINITE:
            return inf;
        case FP_ZERO:
            return sign ? -zero : zero;
        default:
            static_cast<void>(lhs);
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    normalize(sig_lhs, exp_lhs);

    detail::decimal32_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    detail::decimal32_components rhs_components {detail::make_positive_unsigned(rhs), 0, rhs < 0};
    detail::decimal32_components q_components {};

    generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal32(q_components.sig, q_components.exp, q_components.sign);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    // Check pre-conditions
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 nan {boost::decimal::from_bits(boost::decimal::detail::snan_mask)};

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
            return nan;
        default:
            static_cast<void>(lhs);
    }

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    normalize(sig_rhs, exp_rhs);

    detail::decimal32_components lhs_components {detail::make_positive_unsigned(lhs), 0, lhs < 0};
    detail::decimal32_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};
    detail::decimal32_components q_components {};

    generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal32(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto decimal32::operator/=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this / rhs;
    return *this;
}

constexpr auto operator%(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    decimal32 q {};
    decimal32 r {};
    div_impl(lhs, rhs, q, r);
    mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal32::operator%=(boost::decimal::decimal32 rhs) noexcept -> decimal32&
{
    *this = *this % rhs;
    return *this;
}

auto debug_pattern(decimal32 rhs) noexcept -> void
{
    std::cerr << "Sig: " << rhs.full_significand()
              << "\nExp: " << rhs.biased_exponent()
              << "\nNeg: " << rhs.isneg() << std::endl;
}

template <typename T>
BOOST_DECIMAL_CXX20_CONSTEXPR auto decimal32::floating_conversion_impl() const noexcept -> T
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
auto operator>>(std::basic_istream<charT, traits>& is, decimal32& d) -> std::basic_istream<charT, traits>&
{
    char buffer[1024] {}; // What should be an unreasonably high maximum
    is >> buffer;

    bool sign {};
    std::uint64_t significand {};
    std::int32_t expval {};
    const auto buffer_len {std::strlen(buffer)};

    if (buffer_len == 0)
    {
        errno = EINVAL;
        return is;
    }

    const auto r {detail::parser(buffer, buffer + buffer_len, sign, significand, expval)};

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
        d = decimal32(significand, expval, sign);
    }

    return is;
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd32(decimal32 lhs, decimal32 rhs) noexcept -> bool
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

    return lhs.unbiased_exponent() == rhs.unbiased_exponent();
}

// 3.6.5
// Effects: if x is finite, returns its quantum exponent.
// Otherwise, a domain error occurs and INT_MIN is returned.
constexpr auto quantexpd32(decimal32 x) noexcept -> int
{
    if (!isfinite(x))
    {
        return INT_MIN;
    }

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
constexpr auto quantized32(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
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

constexpr auto strtod32(const char* str, char** endptr) noexcept -> decimal32
{
    if (str == nullptr)
    {
        errno = EINVAL;
        return boost::decimal::from_bits(boost::decimal::detail::snan_mask);
    }

    auto sign        = bool {};
    auto significand = std::uint64_t {};
    auto expval      = std::int32_t {};

    const auto buffer_len {detail::strlen(str)};

    if (buffer_len == 0)
    {
        errno = EINVAL;
        return from_bits(boost::decimal::detail::snan_mask);
    }

    const auto r {detail::parser(str, str + buffer_len, sign, significand, expval)};
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
        d = decimal32(significand, expval, sign);
    }

    *endptr = const_cast<char*>(str + (r.ptr - str));
    return d;
}

constexpr auto wcstod32(const wchar_t* str, wchar_t** endptr) noexcept -> decimal32
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
        if (BOOST_DECIMAL_UNLIKELY(val > 255))
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

constexpr auto floord32(decimal32 val) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 neg_one {1, 0, true};
    const auto fp {fpclassify(val)};

    switch (fp)
    {
        case FP_ZERO:
        case FP_NAN:
        case FP_INFINITE:
            return val;
        default:
            static_cast<void>(val);
    }

    auto new_sig {val.full_significand()};

    auto abs_exp = val.biased_exponent(); if (abs_exp < 0) { abs_exp = -abs_exp; }

    const auto sig_dig {detail::num_digits(new_sig)};
    auto decimal_digits {sig_dig};
    bool round {false};

    if (sig_dig > abs_exp)
    {
        decimal_digits = abs_exp;
        if (sig_dig == abs_exp + 1)
        {
            round = true;
        }
    }
    else if (val.biased_exponent() < 1 && abs_exp >= sig_dig)
    {
        return val.isneg() ? neg_one : zero;
    }
    else
    {
        decimal_digits--;
    }

    new_sig /= detail::pow10<std::uint32_t>(decimal_digits);
    if (val.isneg() && round)
    {
        ++new_sig;
    }

    return {new_sig, val.biased_exponent() + decimal_digits, val.isneg()};
}

constexpr auto ceild32(decimal32 val) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 one {1, 0};
    const auto fp {fpclassify(val)};

    switch (fp)
    {
        case FP_ZERO:
        case FP_NAN:
        case FP_INFINITE:
            return val;
        default:
            static_cast<void>(val);
    }

    auto new_sig {val.full_significand()};
    auto abs_exp {std::abs(val.biased_exponent())};
    const auto sig_dig {detail::num_digits(new_sig)};
    auto decimal_digits {sig_dig};

    if (sig_dig > abs_exp)
    {
        decimal_digits = abs_exp;
    }
    else if (val.biased_exponent() < 1 && abs_exp >= sig_dig)
    {
        return val.isneg() ? zero : one;
    }
    else
    {
        decimal_digits--;
    }

    new_sig /= detail::pow10<std::uint32_t>(decimal_digits);
    if (!val.isneg())
    {
        ++new_sig;
    }
    new_sig *= 10;

    return {new_sig, val.biased_exponent() + decimal_digits - 1, val.isneg()};
}

constexpr auto fmodd32(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return lhs % rhs;
}

// Returns the normalized significand and exponent to be cohort agnostic
// Returns num in the range [1'000'000, 9'999'999]
//
// If the conversion can not be performed returns -1 and exp = 0
constexpr auto frexp10d32(decimal32 num, int* expptr) noexcept -> std::int32_t
{
    constexpr decimal32 zero {0, 0};

    if (num == zero)
    {
        *expptr = 0;
        return 0;
    }
    else if (isinf(num) || isnan(num))
    {
        *expptr = 0;
        return -1;
    }

    auto num_exp {num.biased_exponent()};
    auto num_sig {num.full_significand()};
    normalize(num_sig, num_exp);

    *expptr = num_exp;
    auto signed_sig {static_cast<std::int32_t>(num_sig)};
    signed_sig = num.isneg() ? -signed_sig : signed_sig;

    return signed_sig;
}

constexpr auto scalblnd32(decimal32 num, long exp) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};

    if (num == zero || exp == 0 || isinf(num) || isnan(num))
    {
        return num;
    }

    num.edit_exponent(num.biased_exponent() + exp);

    return num;
}

constexpr auto scalbnd32(decimal32 num, int expval) noexcept -> decimal32
{
    return scalblnd32(num, static_cast<long>(expval));
}

constexpr auto copysignd32(decimal32 mag, decimal32 sgn) noexcept -> decimal32
{
    mag.edit_sign(sgn.isneg());
    return mag;
}

constexpr auto fmad32(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return (x * y) + z;
}

constexpr auto modfd32(decimal32 x, decimal32* iptr) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};

    if (abs(x) == zero || isinf(x))
    {
        *iptr = x;
        return x.isneg() ? -zero : zero;
    }
    else if (isnan(x))
    {
        *iptr = x;
        return x;
    }

    *iptr = (x > zero) ? floord32(x) : ceild32(x);
    return (x - *iptr);
}

} // namespace decimal
} // namespace boost

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

    // These members were deprecated in C++23
    #if __cplusplus <= 202002L || _MSVC_LANG <= 202002L
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_denorm_loss = true;
    #endif

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
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (min)        () -> boost::decimal::decimal32 { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (max)        () -> boost::decimal::decimal32 { return {9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto lowest       () -> boost::decimal::decimal32 { return {-9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto epsilon      () -> boost::decimal::decimal32 { return {1, -7}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto round_error  () -> boost::decimal::decimal32 { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto infinity     () -> boost::decimal::decimal32 { return boost::decimal::from_bits(boost::decimal::detail::inf_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto quiet_NaN    () -> boost::decimal::decimal32 { return boost::decimal::from_bits(boost::decimal::detail::nan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto signaling_NaN() -> boost::decimal::decimal32 { return boost::decimal::from_bits(boost::decimal::detail::snan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto denorm_min   () -> boost::decimal::decimal32 { return {1, boost::decimal::detail::etiny}; }
};

} // Namespace std

#endif // BOOST_DECIMAL_DECIMAL32_HPP
