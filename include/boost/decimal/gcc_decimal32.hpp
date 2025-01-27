// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_GCC_DECIMAL32_HPP
#define BOOST_DECIMAL_GCC_DECIMAL32_HPP

#if __has_include(<decimal/decimal>) && defined(__GNUC__) && __GNUC__ >= 7

#include <decimal/decimal>
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE

#include <cstdint>
#include <cstring>

#endif // BOOST_DECIMAL_BUILD_MODULE

#define BOOST_DECIMAL_HAS_GCC_DECIMAL32 1

namespace boost {
namespace decimal {

namespace detail {

// Masks to update the significand based on the combination field
// GCC / IBM do not follow the IEEE 754 encoding standard for BID
// Reverse engineering found the following patters:
//
//    Comb.  Exponent          Significand
// s         eeeeeeee     ttttttttttttttttttttttt - sign + 2 steering bits concatenate to 6 bits of exponent + 23 bits of significand like float
// s   11    eeeeeeee    [t]ttttttttttttttttttttt - sign + 2 steering bits + 8 bits of exponent + 21 bits of significand
//
// Only is the type different in steering 11 which yields significand 100 + 21 bits giving us our 24 total bits of precision

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_sign_mask =          UINT32_C(0b10000000000000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_steering_bits_mask = UINT32_C(0b01100000000000000000000000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_00_steering_bits = UINT32_C(0);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_01_steering_bits = UINT32_C(0b00100000000000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_10_steering_bits = UINT32_C(0b01000000000000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_11_steering_bits = gccd32_steering_bits_mask;

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_01_exp_mask = UINT32_C(0b01111111100000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_11_exp_mask = UINT32_C(0b00011111111000000000000000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_01_significand_mask = UINT32_C(0b00000000011111111111111111111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_11_significand_mask = UINT32_C(0b00000000000111111111111111111111);
} // namespace detail

// This type is a wrapper around gcc std::decimal::decimal32 to allow it to use
// the standard library provided by Boost.Decimal
BOOST_DECIMAL_EXPORT class gcc_decimal32 final
{
public:
    using significand_type = std::uint32_t;
    using exponent_type = std::uint32_t;
    using biased_exponent_type = std::int32_t;

private:

    static_assert(sizeof(std::decimal::decimal32) == sizeof(std::uint32_t), "Incorrect size detected. std::decimal::decimal32 must be exactly 32 bits");

    std::decimal::decimal32 internal_decimal_ {};

#ifdef BOOST_DECIMAL_DEBUG_ACCESSORS
public:
#endif
    // Returns the un-biased (quantum) exponent
    inline auto unbiased_exponent() const noexcept -> exponent_type;

    // Returns the biased exponent
    inline auto biased_exponent() const noexcept -> biased_exponent_type;

    // Returns the significand complete with the bits implied from the combination field
    inline auto full_significand() const noexcept -> significand_type;

    inline auto isneg() const noexcept -> bool;

    // Since we have the overhead of memcpy in order to decode anything
    // get them all at once if we are going to need it
    inline auto to_components() const noexcept -> detail::decimal32_components;

    inline auto underlying() const noexcept -> std::decimal::decimal32 { return internal_decimal_; }

public:

    using return_type = std::decimal::decimal32;

    gcc_decimal32() = default;

    explicit gcc_decimal32(std::decimal::decimal32 decimal) { internal_decimal_ = decimal; }

    // 3.2.5  Initialization from coefficient and exponent.
    gcc_decimal32(long long coeff, int exp);
    gcc_decimal32(unsigned long long coeff, int exp);

    // Non-conforming extension: Conversion to integral type.
    inline operator long long() const noexcept;

    // 3.2.6  Conversion to generic floating-point type.
    inline explicit operator float() const noexcept;
    inline explicit operator double() const noexcept;
    inline explicit operator long double() const noexcept;

    // 3.2.7  Unary arithmetic operators.
    friend return_type operator+(const gcc_decimal32 rhs) { return rhs.underlying(); }
    friend return_type operator-(const gcc_decimal32 rhs) { return -rhs.underlying(); }

    // 3.2.8  Binary arithmetic operators.
    friend auto operator+(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        {return lhs.underlying() + rhs.underlying(); }

    template <typename Integral>
    friend auto operator+(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs.underlying() + rhs; }

    template <typename Integral>
    friend auto operator+(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs + rhs.underlying(); }

    friend auto operator-(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        {return lhs.underlying() - rhs.underlying(); }

    template <typename Integral>
    friend auto operator-(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs.underlying() - rhs; }

    template <typename Integral>
    friend auto operator-(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs - rhs.underlying(); }

    friend auto operator*(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        {return lhs.underlying() * rhs.underlying(); }

    template <typename Integral>
    friend auto operator*(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs.underlying() * rhs; }

    template <typename Integral>
    friend auto operator*(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs * rhs.underlying(); }

    friend auto operator/(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        {return lhs.underlying() / rhs.underlying(); }

    template <typename Integral>
    friend auto operator/(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs.underlying() / rhs; }

    template <typename Integral>
    friend auto operator/(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, return_type)
            { return lhs / rhs.underlying(); }

    // 3.2.9  Comparison operators.
    friend auto operator==(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        {return lhs.underlying() == rhs.underlying(); }

    template <typename Integral>
    friend auto operator==(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() + rhs; }

    template <typename Integral>
    friend auto operator==(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs + rhs.underlying(); }

    friend auto operator!=(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        { return lhs.underlying() != rhs.underlying(); }

    template <typename Integral>
    friend auto operator!=(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() + rhs; }

    template <typename Integral>
    friend auto operator!=(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs + rhs.underlying(); }

    friend auto operator<(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        {return lhs.underlying() < rhs.underlying(); }

    template <typename Integral>
    friend auto operator<(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() + rhs; }

    template <typename Integral>
    friend auto operator<(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs + rhs.underlying(); }

    friend auto operator<=(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        { return lhs.underlying() <= rhs.underlying(); }

    template <typename Integral>
    friend auto operator<=(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() + rhs; }

    template <typename Integral>
    friend auto operator<=(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs + rhs.underlying(); }

    friend auto operator>(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        { return lhs.underlying() > rhs.underlying(); }

    template <typename Integral>
    friend auto operator>(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() + rhs; }

    template <typename Integral>
    friend auto operator>(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs + rhs.underlying(); }

    friend auto operator>=(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> return_type
        { return lhs.underlying() >= rhs.underlying(); }

    template <typename Integral>
    friend auto operator>=(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() + rhs; }

    template <typename Integral>
    friend auto operator>=(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs + rhs.underlying(); }
};

namespace detail {

inline auto decode_gccd32_unbiased_exponent(std::uint32_t bits_) noexcept -> gcc_decimal32::exponent_type
{
    gcc_decimal32::exponent_type expval {};
    const auto steering_bits {(bits_ & gccd32_steering_bits_mask)};

    // 3 of the 4 steering patterns leave this laid out like a binary float instead of an IEEE 754 decimal float
    if (steering_bits != gccd32_11_steering_bits)
    {
        expval |= (bits_ & gccd32_01_exp_mask) >> 23U;
    }
    else
    {
        expval |= (bits_ & gccd32_11_exp_mask) >> 21U;
    }

    return expval;
}

inline auto decode_gccd32_biased_exponent(std::uint32_t bits) -> gcc_decimal32::biased_exponent_type
{
    return static_cast<gcc_decimal32::biased_exponent_type>(decode_gccd32_unbiased_exponent(bits)) - detail::bias_v<decimal32>;
}

inline auto decode_gccd32_significand(std::uint32_t bits_) -> gcc_decimal32::significand_type
{
    gcc_decimal32::significand_type significand {};

    const auto steering_bits {(bits_ & gccd32_steering_bits_mask)};

    if (steering_bits != gccd32_11_steering_bits)
    {
        significand |= (bits_ & gccd32_01_significand_mask);
    }
    else
    {
        significand = UINT32_C(0b1000'0000000000'0000000000);
        significand |= (bits_ & gccd32_11_significand_mask);
    }

    return significand;
}

inline auto decode_gccd32_sign(std::uint32_t bits_) -> bool
{
    return static_cast<bool>(bits_ & gccd32_sign_mask);
}

} // detail

inline auto gcc_decimal32::unbiased_exponent() const noexcept -> exponent_type
{
    std::uint32_t bits_;
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_gccd32_unbiased_exponent(bits_);
}

inline auto gcc_decimal32::biased_exponent() const noexcept -> biased_exponent_type
{
    std::uint32_t bits_;
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_gccd32_biased_exponent(bits_);
}

inline auto gcc_decimal32::full_significand() const noexcept -> significand_type
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_gccd32_significand(bits_);
}

inline auto gcc_decimal32::isneg() const noexcept -> bool
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_gccd32_sign(bits_);
}

inline auto gcc_decimal32::to_components() const noexcept -> detail::decimal32_components
{
    detail::decimal32_components components {};
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));

    gcc_decimal32::exponent_type expval {};
    gcc_decimal32::significand_type significand {};
    const auto steering_bits {(bits_ & detail::gccd32_steering_bits_mask)};

    if (steering_bits != detail::gccd32_11_steering_bits)
    {
        significand |= (bits_ & detail::gccd32_01_significand_mask);
        expval |= (bits_ & detail::gccd32_01_exp_mask) >> 23U;
    }
    else
    {
        significand = UINT32_C(0b1000'0000000000'0000000000);
        significand |= (bits_ & detail::gccd32_11_significand_mask);
        expval |= (bits_ & detail::gccd32_11_exp_mask) >> 21U;
    }

    components.sig = significand;
    components.exp = static_cast<gcc_decimal32::biased_exponent_type>(expval) - detail::bias_v<decimal32>;
    components.sign = bits_ & detail::gccd32_sign_mask;

    return components;
}

inline gcc_decimal32::gcc_decimal32(long long coeff, int exp)
{
    internal_decimal_ = std::decimal::make_decimal32(coeff, exp);
}

inline gcc_decimal32::gcc_decimal32(unsigned long long coeff, int exp)
{
    internal_decimal_ = std::decimal::make_decimal32(coeff, exp);
}

inline gcc_decimal32::operator long long() const noexcept
{
    return std::decimal::decimal32_to_long_long(internal_decimal_);
}

inline gcc_decimal32::operator float() const noexcept
{
    return std::decimal::decimal32_to_float(internal_decimal_);
}

inline gcc_decimal32::operator double() const noexcept
{
    return std::decimal::decimal32_to_double(internal_decimal_);
}

inline gcc_decimal32::operator long double() const noexcept
{
    return std::decimal::decimal32_to_long_double(internal_decimal_);
}

} // namespace decimal
} // namespace boost

#else

#error "libstdc++ header <decimal/decimal> is required to use this functionality"

#endif // __has_include(<decimal/decimal>)

#endif // BOOST_DECIMAL_GCC_DECIMAL32_HPP
