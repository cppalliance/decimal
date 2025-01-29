// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_GCC_DECIMAL32_HPP
#define BOOST_DECIMAL_GCC_DECIMAL32_HPP

#include <boost/decimal/detail/config.hpp>

#ifdef BOOST_DECIMAL_HAS_LIBSTDCPP_DECIMAL

#include <decimal/decimal>
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/detail/to_integral.hpp>

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

#ifdef BOOST_DECIMAL_ENDIAN_LITTLE_BYTE

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

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_inf_mask = UINT32_C(0x78000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_qnan_mask = UINT32_C(0x7C000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_snan_mask = UINT32_C(0x7E000000);

#else

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_sign_mask =          UINT32_C(0b00000000000000000000000000000001);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_steering_bits_mask = UINT32_C(0b00000000000000000000000000000110);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_00_steering_bits = UINT32_C(0);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_01_steering_bits = UINT32_C(0b00000000000000000000000000000100);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_10_steering_bits = UINT32_C(0b00000000000000000000000000000010);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_11_steering_bits = gccd32_steering_bits_mask;

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_01_exp_mask = UINT32_C(0b00000000000000000000000111111110);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_11_exp_mask = UINT32_C(0b00000000000000000000011111111000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_01_significand_mask = UINT32_C(0b11111111111111111111111000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_11_significand_mask = UINT32_C(0b11111111111111111111100000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_inf_mask  = UINT32_C(0b00000000000000000000000000011110);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_qnan_mask = UINT32_C(0b00000000000000000000000000111110);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_snan_mask = UINT32_C(0b00000000000000000000000001111110);

#endif

// Non-finite values
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_inf = UINT32_C(0x78000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_qnan = UINT32_C(0x7C000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t gccd32_snan = UINT32_C(0x7E000000);

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

    // cmath functions that are easier as friends
    friend inline auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool;
    friend inline auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool;
    friend inline auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool;
    friend inline auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool;
    friend inline auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool;
    friend inline auto isfinite    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_integral_v, TargetType, TargetType);

public:

    gcc_decimal32() = default;

    explicit gcc_decimal32(std::decimal::decimal32 decimal) { internal_decimal_ = decimal; }

    // 3.2.5  Initialization from coefficient and exponent.
    template <BOOST_DECIMAL_INTEGRAL Integer, std::enable_if_t<detail::is_integral_v<Integer> && detail::is_signed_v<Integer>, bool> = true>
    gcc_decimal32(Integer coeff)
    {
        internal_decimal_ = std::decimal::make_decimal32(static_cast<long long>(coeff), 0);
    }

    template <BOOST_DECIMAL_INTEGRAL Integer, std::enable_if_t<detail::is_integral_v<Integer> && detail::is_unsigned_v<Integer>, bool> = true>
    gcc_decimal32(Integer coeff)
    {
        internal_decimal_ = std::decimal::make_decimal32(static_cast<unsigned long long>(coeff), 0);
    }

    template <BOOST_DECIMAL_INTEGRAL Integer, std::enable_if_t<detail::is_integral_v<Integer> && detail::is_signed_v<Integer>, bool> = true>
    gcc_decimal32(Integer coeff, int exp)
    {
        internal_decimal_ = std::decimal::make_decimal32(static_cast<long long>(coeff), exp);
    }

    template <BOOST_DECIMAL_INTEGRAL Integer, std::enable_if_t<detail::is_integral_v<Integer> && detail::is_unsigned_v<Integer>, bool> = true>
    gcc_decimal32(Integer coeff, int exp)
    {
        internal_decimal_ = std::decimal::make_decimal32(static_cast<unsigned long long>(coeff), exp);
    }

    // Non-conforming extension: Conversion to integral type.
    inline operator unsigned long long() const noexcept;
    inline operator long long() const noexcept;
    inline operator unsigned long() const noexcept;
    inline operator long() const noexcept;
    inline operator unsigned() const noexcept;
    inline operator int() const noexcept;

    // 3.2.6  Conversion to generic floating-point type.
    inline explicit operator float() const noexcept;
    inline explicit operator double() const noexcept;
    inline explicit operator long double() const noexcept;

    // 3.2.7  Unary arithmetic operators.
    friend gcc_decimal32 operator+(const gcc_decimal32 rhs) { return gcc_decimal32{rhs.underlying()}; }
    friend gcc_decimal32 operator-(const gcc_decimal32 rhs) { return gcc_decimal32{-rhs.underlying()}; }

    // 3.2.8  Binary arithmetic operators.
    friend auto operator+(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> gcc_decimal32
        {return gcc_decimal32{lhs.underlying() + rhs.underlying()}; }

    template <typename Integral>
    friend auto operator+(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs.underlying() + rhs}; }

    template <typename Integral>
    friend auto operator+(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs + rhs.underlying()}; }

    friend auto operator-(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> gcc_decimal32
        {return gcc_decimal32{lhs.underlying() - rhs.underlying()}; }

    template <typename Integral>
    friend auto operator-(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs.underlying() - rhs}; }

    template <typename Integral>
    friend auto operator-(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs - rhs.underlying()}; }

    friend auto operator*(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> gcc_decimal32
        { return gcc_decimal32{lhs.underlying() * rhs.underlying()}; }

    template <typename Integral>
    friend auto operator*(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs.underlying() * rhs}; }

    template <typename Integral>
    friend auto operator*(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs * rhs.underlying()}; }

    friend auto operator/(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> gcc_decimal32
        {return gcc_decimal32{lhs.underlying() / rhs.underlying()}; }

    template <typename Integral>
    friend auto operator/(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs.underlying() / rhs}; }

    template <typename Integral>
    friend auto operator/(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, gcc_decimal32)
            { return gcc_decimal32{lhs / rhs.underlying()}; }

    // 3.2.9  Comparison operators.
    friend auto operator==(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> bool
        { return lhs.underlying() == rhs.underlying(); }

    template <typename Integral>
    friend auto operator==(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() == rhs; }

    template <typename Integral>
    friend auto operator==(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs == rhs.underlying(); }

    friend auto operator!=(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> bool
        { return lhs.underlying() != rhs.underlying(); }

    template <typename Integral>
    friend auto operator!=(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() != rhs; }

    template <typename Integral>
    friend auto operator!=(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs != rhs.underlying(); }

    friend auto operator<(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> bool
        {return lhs.underlying() < rhs.underlying(); }

    template <typename Integral>
    friend auto operator<(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() < rhs; }

    template <typename Integral>
    friend auto operator<(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs < rhs.underlying(); }

    friend auto operator<=(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> bool
        { return lhs.underlying() <= rhs.underlying(); }

    template <typename Integral>
    friend auto operator<=(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() <= rhs; }

    template <typename Integral>
    friend auto operator<=(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs <= rhs.underlying(); }

    friend auto operator>(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> bool
        { return lhs.underlying() > rhs.underlying(); }

    template <typename Integral>
    friend auto operator>(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() > rhs; }

    template <typename Integral>
    friend auto operator>(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs > rhs.underlying(); }

    friend auto operator>=(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> bool
        { return lhs.underlying() >= rhs.underlying(); }

    template <typename Integral>
    friend auto operator>=(gcc_decimal32 lhs, Integral rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs.underlying() >= rhs; }

    template <typename Integral>
    friend auto operator>=(Integral lhs, gcc_decimal32 rhs)
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integral, bool)
            { return lhs >= rhs.underlying(); }

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(gcc_decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, gcc_decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    // 3.2.2.6  Increment and decrement operators
    inline auto operator++() noexcept -> gcc_decimal32&;
    inline auto operator++(int) noexcept -> gcc_decimal32;
    inline auto operator--() noexcept -> gcc_decimal32&;
    inline auto operator--(int) noexcept -> gcc_decimal32;

    // 3.2.2.7  Compound assignment.
    inline auto operator+=(gcc_decimal32 rhs) noexcept -> gcc_decimal32&
        { internal_decimal_ += rhs.underlying(); return *this; }

    template <typename Integer>
    inline auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, gcc_decimal32&)
            { internal_decimal_ += rhs; return *this; }

    inline auto operator-=(gcc_decimal32 rhs) noexcept -> gcc_decimal32&
        { internal_decimal_ -= rhs.underlying(); return *this; }

    template <typename Integer>
    inline auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, gcc_decimal32&)
            { internal_decimal_ -= rhs; return *this; }

    template <typename Integer>
    inline auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, gcc_decimal32&)
            { internal_decimal_ *= rhs; return *this; }

    inline auto operator*=(gcc_decimal32 rhs) noexcept -> gcc_decimal32&
        { internal_decimal_ *= rhs.underlying(); return *this; }

    template <typename Integer>
    inline auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, gcc_decimal32&)
            { internal_decimal_ /= rhs; return *this; }

    inline auto operator/=(gcc_decimal32 rhs) noexcept -> gcc_decimal32&
        { internal_decimal_ /= rhs.underlying(); return *this; }

    // A basic output operator for now
    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, const gcc_decimal32& d) -> std::basic_ostream<charT, traits>&
    {
        if (d.isneg())
        {
            os << "-";
        }
        else
        {
            os << "+";
        }

        os << d.full_significand();

        const auto exp {d.biased_exponent()};

        if (exp < 0)
        {
            os << "e-" << exp;
        }
        else
        {
            os << "e+" << exp;
        }

        return os;
    }
};

template <typename charT, typename traits>
auto operator<<(std::basic_ostream<charT, traits>& os, const std::decimal::decimal32& d) -> std::basic_ostream<charT, traits>&
{
    return os << gcc_decimal32{d};
}

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

inline auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool
{
    return rhs.isneg();
}

inline auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &rhs.internal_decimal_, sizeof(std::uint32_t));

    return bits_ == detail::gccd32_inf;
}

inline auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &rhs.internal_decimal_, sizeof(std::uint32_t));

    return bits_ >= detail::gccd32_qnan;
}

inline auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &rhs.internal_decimal_, sizeof(std::uint32_t));

    return bits_ == detail::gccd32_snan;
}

inline auto isfinite BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &rhs.internal_decimal_, sizeof(std::uint32_t));

    return bits_ < detail::gccd32_inf;
}

inline auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (gcc_decimal32 rhs) noexcept -> bool
{
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision_v<decimal32> - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
}

inline gcc_decimal32::operator unsigned long long() const noexcept
{
    return to_integral<gcc_decimal32, unsigned long long>(*this);
}

inline gcc_decimal32::operator long long() const noexcept
{
    return std::decimal::decimal32_to_long_long(internal_decimal_);
}

inline gcc_decimal32::operator unsigned long() const noexcept
{
    return to_integral<gcc_decimal32, unsigned long>(*this);
}

inline gcc_decimal32::operator long() const noexcept
{
    return static_cast<long>(std::decimal::decimal32_to_long_long(internal_decimal_));
}

inline gcc_decimal32::operator unsigned() const noexcept
{
    return static_cast<unsigned>(std::decimal::decimal32_to_long_long(internal_decimal_));
}

inline gcc_decimal32::operator int() const noexcept
{
    return static_cast<int>(std::decimal::decimal32_to_long_long(internal_decimal_));
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

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

constexpr auto operator<=>(gcc_decimal32 lhs, gcc_decimal32 rhs) noexcept -> std::partial_ordering
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
constexpr auto operator<=>(gcc_decimal32 lhs, Integer rhs) noexcept
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
constexpr auto operator<=>(Integer lhs, gcc_decimal32 rhs) noexcept
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

inline auto gcc_decimal32::operator++() noexcept -> gcc_decimal32&
{
    internal_decimal_++;
    return *this;
}

inline auto gcc_decimal32::operator++(int) noexcept -> gcc_decimal32
{
    internal_decimal_++;
    return *this;
}

inline auto gcc_decimal32::operator--() noexcept -> gcc_decimal32&
{
    internal_decimal_--;
    return *this;
}

inline auto gcc_decimal32::operator--(int) noexcept -> gcc_decimal32
{
    internal_decimal_--;
    return *this;
}

} // namespace decimal
} // namespace boost

BOOST_DECIMAL_EXPORT template <>
struct std::numeric_limits<boost::decimal::gcc_decimal32>
{
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = false;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = true;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    static constexpr bool has_denorm_loss = true;
    #endif

    static constexpr std::float_round_style round_style = std::round_indeterminate;
    static constexpr bool is_iec559 = true;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    static constexpr int digits = 7;
    static constexpr int digits10 = digits;
    static constexpr int max_digits10 = digits;
    static constexpr int radix = 10;
    static constexpr int min_exponent = -95;
    static constexpr int min_exponent10 = min_exponent;
    static constexpr int max_exponent = 96;
    static constexpr int max_exponent10 = max_exponent;
    static constexpr bool traps = numeric_limits<std::uint32_t>::traps;
    static constexpr bool tinyness_before = true;

    // Member functions
    static inline auto (min)        () -> boost::decimal::gcc_decimal32 { return {1, min_exponent}; }
    static inline auto (max)        () -> boost::decimal::gcc_decimal32 { return {9'999'999, max_exponent - digits + 1}; }
    static inline auto lowest       () -> boost::decimal::gcc_decimal32 { return {-9'999'999, max_exponent - digits + 1}; }
    static inline auto epsilon      () -> boost::decimal::gcc_decimal32 { return {1, -digits + 1}; }
    static inline auto round_error  () -> boost::decimal::gcc_decimal32 { return epsilon(); }

    #if defined(__GNUC__) && __GNUC__ >= 8
    #  pragma GCC diagnostic push
    #  pragma GCC diagnostic ignored "-Wclass-memaccess"
    #endif

    static inline auto infinity     () -> boost::decimal::gcc_decimal32
    {
        static_assert(sizeof(std::decimal::decimal32) == sizeof(std::uint32_t), "Decimal32 is an unexpected size.");
        constexpr std::uint32_t bits_ = boost::decimal::detail::gccd32_inf_mask;
        std::decimal::decimal32 val {};
        std::memcpy(&val, &bits_, sizeof(std::uint32_t));
        return boost::decimal::gcc_decimal32{val};
    }

    static inline auto quiet_NaN    () -> boost::decimal::gcc_decimal32
    {
        static_assert(sizeof(std::decimal::decimal32) == sizeof(std::uint32_t), "Decimal32 is an unexpected size.");
        constexpr std::uint32_t bits_ = boost::decimal::detail::gccd32_qnan_mask;
        std::decimal::decimal32 val {};
        std::memcpy(&val, &bits_, sizeof(std::uint32_t));
        return boost::decimal::gcc_decimal32{val};
    }

    static inline auto signaling_NaN() -> boost::decimal::gcc_decimal32
    {
        static_assert(sizeof(std::decimal::decimal32) == sizeof(std::uint32_t), "Decimal32 is an unexpected size.");
        constexpr std::uint32_t bits_ = boost::decimal::detail::gccd32_snan_mask;
        std::decimal::decimal32 val {};
        std::memcpy(&val, &bits_, sizeof(std::uint32_t));
        return boost::decimal::gcc_decimal32{val};
    }

    #if defined(__GNUC__) && __GNUC__ >= 8
    #  pragma GCC diagnostic pop
    #endif

    static inline auto denorm_min   () -> boost::decimal::gcc_decimal32 { return {1, boost::decimal::detail::etiny}; }
};

#else

#error "libstdc++ header <decimal/decimal> is required to use this functionality"

#endif // __has_include(<decimal/decimal>)

#endif // BOOST_DECIMAL_GCC_DECIMAL32_HPP
