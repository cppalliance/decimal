// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_GCC_DECIMAL32_HPP
#define BOOST_DECIMAL_GCC_DECIMAL32_HPP

#if __has_include(<decimal/decimal>)

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

// This type is a wrapper around gcc std::decimal::decimal32 to allow it to use
// the standard library provided by Boost.Decimal
BOOST_DECIMAL_EXPORT class gcc_decimal32 final
{
public:
    using significand_type = std::uint32_t;
    using exponent_type = std::uint32_t;
    using biased_exponent_type = std::int32_t;

private:

    std::decimal::decimal32 internal_decimal_ {};

    // Returns the un-biased (quantum) exponent
    BOOST_DECIMAL_NO_DISCARD inline auto unbiased_exponent() const noexcept -> exponent_type;

    // Returns the biased exponent
    BOOST_DECIMAL_NO_DISCARD inline auto biased_exponent() const noexcept -> biased_exponent_type;

    // Returns the significand complete with the bits implied from the combination field
    BOOST_DECIMAL_NO_DISCARD inline auto full_significand() const noexcept -> significand_type;

    BOOST_DECIMAL_NO_DISCARD inline auto isneg() const noexcept -> bool;

    // Since we have the overhead of memcpy in order to decode anything
    // get them all at once if we are going to need it
    BOOST_DECIMAL_NO_DISCARD inline auto to_components() const noexcept -> detail::decimal32_components;

public:

    gcc_decimal32() = default;

    // 3.2.5  Initialization from coefficient and exponent.
    gcc_decimal32(long long coeff, int exp);
    gcc_decimal32(unsigned long long coeff, int exp);

    // Non-conforming extension: Conversion to integral type.
    inline operator long long() const noexcept;

    // 3.2.6  Conversion to generic floating-point type.
    inline explicit operator float() const noexcept;
    inline explicit operator double() const noexcept;
    inline explicit operator long double() const noexcept;

};

namespace detail {

BOOST_DECIMAL_NO_DISCARD inline auto decode_gccd32_unbiased_exponent(std::uint32_t bits_) noexcept -> gcc_decimal32::exponent_type
{
    gcc_decimal32::exponent_type expval {};
    const auto exp_comb_bits {(bits_ & detail::d32_comb_11_mask)};

    switch (exp_comb_bits)
    {
        case detail::d32_comb_11_mask:
            // bits 2 and 3 are the exp part of the combination field
                expval = (bits_ & detail::d32_comb_11_exp_bits) >> (detail::d32_significand_bits + 1);
        break;
        case detail::d32_comb_10_mask:
            expval = UINT32_C(0b10000000);
        break;
        case detail::d32_comb_01_mask:
            expval = UINT32_C(0b01000000);
        break;
        // LCOV_EXCL_START
        default:
            BOOST_DECIMAL_UNREACHABLE;
        // LCOV_EXCL_STOP
    }

    expval |= (bits_ & detail::d32_exponent_mask) >> detail::d32_significand_bits;

    return expval;
}

BOOST_DECIMAL_NO_DISCARD inline auto decode_gccd32_biased_exponent(std::uint32_t bits) -> gcc_decimal32::biased_exponent_type
{
    return static_cast<gcc_decimal32::biased_exponent_type>(decode_gccd32_unbiased_exponent(bits)) - detail::bias_v<decimal32>;
}

BOOST_DECIMAL_NO_DISCARD inline auto decode_gccd32_significand(std::uint32_t bits_) -> gcc_decimal32::significand_type
{
    gcc_decimal32::significand_type significand {};

    if ((bits_ & detail::d32_comb_11_mask) == detail::d32_comb_11_mask)
    {
        // Only need the one bit of T because the other 3 are implied
        significand = (bits_ & detail::d32_comb_11_significand_bits) == detail::d32_comb_11_significand_bits ?
                      UINT32_C(0b1001'0000000000'0000000000) :
                      UINT32_C(0b1000'0000000000'0000000000);
    }
    else
    {
        // Last three bits in the combination field, so we need to shift past the exp field
        // which is next
        significand |= (bits_ & detail::d32_comb_00_01_10_significand_bits) >> detail::d32_exponent_bits;
    }

    significand |= (bits_ & detail::d32_significand_mask);

    return significand;
}

BOOST_DECIMAL_NO_DISCARD inline auto decode_gccd32_sign(std::uint32_t bits_) -> bool
{
    return static_cast<bool>(bits_ & detail::d32_sign_mask);
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

    components.sig = detail::decode_gccd32_significand(bits_);
    components.exp = detail::decode_gccd32_biased_exponent(bits_);
    components.sign = detail::decode_gccd32_sign(bits_);

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
