// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_INTEL_DECIMAL32_HPP
#define BOOST_DECIMAL_INTEL_DECIMAL32_HPP

#if __has_include("decimal.h")

#include "decimal.h"
#include "bid_conf.h"
#include "bid_functions.h"
#include <boost/decimal/decimal32.hpp>

#define BOOST_DECIMAL_HAS_INTEL_WRAPPER

namespace boost {
namespace decimal {
namespace detail {

// Masks to update the significand based on the combination field
// Intel does not follow the IEEE 754 encoding standard for BID
// Reverse engineering found the following patters:
//
//    Comb.  Exponent          Significand
// s         eeeeeeee     ttttttttttttttttttttttt - sign + 2 steering bits concatenate to 6 bits of exponent + 23 bits of significand like float
// s   11    eeeeeeee    [t]ttttttttttttttttttttt - sign + 2 steering bits + 8 bits of exponent + 21 bits of significand
//
// Only is the type different in steering 11 which yields significand 100 + 21 bits giving us our 24 total bits of precision

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_sign_mask =          UINT32_C(0b10000000000000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_steering_bits_mask = UINT32_C(0b01100000000000000000000000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_00_steering_bits = UINT32_C(0);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_01_steering_bits = UINT32_C(0b00100000000000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_10_steering_bits = UINT32_C(0b01000000000000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_11_steering_bits = inteld32_steering_bits_mask;

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_01_exp_mask = UINT32_C(0b01111111100000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_11_exp_mask = UINT32_C(0b00011111111000000000000000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_01_significand_mask = UINT32_C(0b00000000011111111111111111111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_11_significand_mask = UINT32_C(0b00000000000111111111111111111111);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_inf_mask = UINT32_C(0x78000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_qnan_mask = UINT32_C(0x7C000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t inteld32_snan_mask = UINT32_C(0x7E000000);

} // namespace detail


class intel_decimal32
{
public:

    using significand_type = std::uint32_t;
    using exponent_type = std::uint32_t;
    using biased_exponent_type = std::int32_t;

private:

    static_assert(sizeof(Decimal32) == sizeof(std::uint32_t), "Decimal32 is the incorrect size");

    Decimal32 internal_decimal_ {};

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

    inline auto underlying() const noexcept -> Decimal32 { return internal_decimal_; }
    

public:

    intel_decimal32() = default;

    // 3.2.5  Initialization from coefficient and exponent.
    explicit intel_decimal32(std::int32_t value)
    {
        internal_decimal_ = bid32_from_int32(value, 0, 0);
    }
};

namespace detail {

inline auto decode_inteld32_unbiased_exponent(std::uint32_t bits_) noexcept -> intel_decimal32::exponent_type
{
    intel_decimal32::exponent_type expval {};
    const auto steering_bits {(bits_ & inteld32_steering_bits_mask)};

    // 3 of the 4 steering patterns leave this laid out like a binary float instead of an IEEE 754 decimal float
    if (steering_bits != inteld32_11_steering_bits)
    {
        expval |= (bits_ & inteld32_01_exp_mask) >> 23U;
    }
    else
    {
        expval |= (bits_ & inteld32_11_exp_mask) >> 21U;
    }

    return expval;
}

inline auto decode_inteld32_biased_exponent(std::uint32_t bits) -> intel_decimal32::biased_exponent_type
{
    return static_cast<intel_decimal32::biased_exponent_type>(decode_inteld32_unbiased_exponent(bits)) - detail::bias_v<decimal32>;
}

inline auto decode_inteld32_significand(std::uint32_t bits_) -> intel_decimal32::significand_type
{
    intel_decimal32::significand_type significand {};

    const auto steering_bits {(bits_ & inteld32_steering_bits_mask)};

    if (steering_bits != inteld32_11_steering_bits)
    {
        significand |= (bits_ & inteld32_01_significand_mask);
    }
    else
    {
        significand = UINT32_C(0b1000'0000000000'0000000000);
        significand |= (bits_ & inteld32_11_significand_mask);
    }

    return significand;
}

inline auto decode_inteld32_sign(std::uint32_t bits_) -> bool
{
    return static_cast<bool>(bits_ & inteld32_sign_mask);
}

} // detail

inline auto intel_decimal32::unbiased_exponent() const noexcept -> exponent_type
{
    std::uint32_t bits_;
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_inteld32_unbiased_exponent(bits_);
}

inline auto intel_decimal32::biased_exponent() const noexcept -> biased_exponent_type
{
    std::uint32_t bits_;
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_inteld32_biased_exponent(bits_);
}

inline auto intel_decimal32::full_significand() const noexcept -> significand_type
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_inteld32_significand(bits_);
}

inline auto intel_decimal32::isneg() const noexcept -> bool
{
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));
    return detail::decode_inteld32_sign(bits_);
}

inline auto intel_decimal32::to_components() const noexcept -> detail::decimal32_components
{
    detail::decimal32_components components {};
    std::uint32_t bits_ {};
    std::memcpy(&bits_, &internal_decimal_, sizeof(std::uint32_t));

    intel_decimal32::exponent_type expval {};
    intel_decimal32::significand_type significand {};
    const auto steering_bits {(bits_ & detail::inteld32_steering_bits_mask)};

    if (steering_bits != detail::inteld32_11_steering_bits)
    {
        significand |= (bits_ & detail::inteld32_01_significand_mask);
        expval |= (bits_ & detail::inteld32_01_exp_mask) >> 23U;
    }
    else
    {
        significand = UINT32_C(0b1000'0000000000'0000000000);
        significand |= (bits_ & detail::inteld32_11_significand_mask);
        expval |= (bits_ & detail::inteld32_11_exp_mask) >> 21U;
    }

    components.sig = significand;
    components.exp = static_cast<intel_decimal32::biased_exponent_type>(expval) - detail::bias_v<decimal32>;
    components.sign = bits_ & detail::inteld32_sign_mask;

    return components;
}

} // namespace decimal
} // namespace boost

#endif // has intel header

#endif // BOOST_DECIMAL_INTEL_DECIMAL32_HPP
