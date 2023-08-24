// Copyright 2018 - 2023 Ulf Adams
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_RYU_RYU_GENERIC_128_HPP
#define BOOST_DECIMAL_DETAIL_RYU_RYU_GENERIC_128_HPP

#include <boost/decimal/detail/ryu/generic_128.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/bit_layouts.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <cinttypes>
#include <cstdio>
#include <cstdint>

#ifdef BOOST_DECIMAL_DEBUG
#  include <iostream>
#endif

namespace boost { namespace decimal { namespace detail { namespace ryu {

static constexpr int32_t fd128_exceptional_exponent = 0x7FFFFFFF;
static constexpr unsigned_128_type one = 1;

struct floating_decimal_128
{
    unsigned_128_type mantissa;
    int32_t exponent;
    bool sign;
};

#ifdef BOOST_DECIMAL_DEBUG
static char* s(unsigned_128_type v) {
  int len = num_digits(v);
  char* b = (char*) malloc((len + 1) * sizeof(char));
  for (int i = 0; i < len; i++) {
    const uint32_t c = (uint32_t) (v % 10);
    v /= 10;
    b[len - 1 - i] = (char) ('0' + c);
  }
  b[len] = 0;
  return b;
}
#endif

static constexpr struct floating_decimal_128 generic_binary_to_decimal(
        const unsigned_128_type bits,
        const std::uint32_t mantissaBits, const std::uint32_t exponentBits, const bool explicitLeadingBit) noexcept
{
    #ifdef BOOST_DECIMAL_DEBUG
    printf("IN=");
    for (int32_t bit = 127; bit >= 0; --bit)
    {
        printf("%u", (uint32_t) ((bits >> bit) & 1));
    }
    printf("\n");
    #endif

    const std::uint32_t bias = (1u << (exponentBits - 1)) - 1;
    const bool ieeeSign = ((bits >> (mantissaBits + exponentBits)) & 1) != 0;
    const unsigned_128_type ieeeMantissa = bits & ((one << mantissaBits) - 1);
    const auto ieeeExponent = (uint32_t) ((bits >> mantissaBits) & ((one << exponentBits) - 1u));

    if (ieeeExponent == 0 && ieeeMantissa == 0)
    {
        struct floating_decimal_128 fd {0, 0, ieeeSign};
        return fd;
    }
    if (ieeeExponent == ((1u << exponentBits) - 1u))
    {
        struct floating_decimal_128 fd {};
        fd.mantissa = explicitLeadingBit ? ieeeMantissa & ((one << (mantissaBits - 1)) - 1) : ieeeMantissa;
        fd.exponent = fd128_exceptional_exponent;
        fd.sign = ieeeSign;
        return fd;
    }

    int32_t e2 {};
    unsigned_128_type m2 {};
    // We subtract 2 in all cases so that the bounds computation has 2 additional bits.
    if (explicitLeadingBit)
    {
        // mantissaBits includes the explicit leading bit, so we need to correct for that here.
        if (ieeeExponent == 0)
        {
            e2 = (int32_t)(1 - bias - mantissaBits + 1 - 2);
        }
        else
        {
            e2 = (int32_t)(ieeeExponent - bias - mantissaBits + 1 - 2);
        }
        m2 = ieeeMantissa;
    }
    else
    {
        if (ieeeExponent == 0)
        {
            e2 = (int32_t)(1 - bias - mantissaBits - 2);
            m2 = ieeeMantissa;
        } else
        {
            e2 = static_cast<int32_t>(ieeeExponent - bias - mantissaBits - 2);
            m2 = (one << mantissaBits) | ieeeMantissa;
        }
    }
    const bool even = (m2 & 1) == 0;
    const bool acceptBounds = even;

    #ifdef BOOST_DECIMAL_DEBUG
    printf("-> %s %s * 2^%d\n", ieeeSign ? "-" : "+", s(m2), e2 + 2);
    #endif

    // Step 2: Determine the interval of legal decimal representations.
    const unsigned_128_type mv = 4 * m2;
    // Implicit bool -> int conversion. True is 1, false is 0.
    const std::uint32_t mmShift =
            (ieeeMantissa != (explicitLeadingBit ? one << (mantissaBits - 1) : 0))
            || (ieeeExponent == 0);

    // Step 3: Convert to a decimal power base using 128-bit arithmetic.
    unsigned_128_type vr {};
    unsigned_128_type vp {};
    unsigned_128_type vm {};
    int32_t e10 {};
    bool vmIsTrailingZeros = false;
    bool vrIsTrailingZeros = false;
    if (e2 >= 0)
    {
        // I tried special-casing q == 0, but there was no effect on performance.
        // This expression is slightly faster than max(0, log10Pow2(e2) - 1).
        const std::uint32_t q = log10Pow2(e2) - (e2 > 3);
        e10 = (int32_t)q;
        const int32_t k = BOOST_DECIMAL_POW5_INV_BITCOUNT + pow5bits(q) - 1;
        const auto i = (int32_t)(-e2 + q + k);
        uint64_t pow5[4] {};
        generic_computeInvPow5(q, pow5);
        vr = mulShift(4 * m2, pow5, i);
        vp = mulShift(4 * m2 + 2, pow5, i);
        vm = mulShift(4 * m2 - 1 - mmShift, pow5, i);

        #ifdef BOOST_DECIMAL_DEBUG
        printf("%s * 2^%d / 10^%d\n", s(mv), e2, q);
        printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
        #endif

        // floor(log_5(2^128)) = 55, this is very conservative
        if (q <= 55)
        {
            // Only one of mp, mv, and mm can be a multiple of 5, if any.
            if (mv % 5 == 0)
            {
                vrIsTrailingZeros = multipleOfPowerOf5(mv, q - 1);
            }
            else if (acceptBounds)
            {
                // Same as min(e2 + (~mm & 1), pow5Factor(mm)) >= q
                // <=> e2 + (~mm & 1) >= q && pow5Factor(mm) >= q
                // <=> true && pow5Factor(mm) >= q, since e2 >= q.
                vmIsTrailingZeros = multipleOfPowerOf5(mv - 1 - mmShift, q);
            }
            else
            {
                // Same as min(e2 + 1, pow5Factor(mp)) >= q.
                vp -= multipleOfPowerOf5(mv + 2, q);
            }
        }
    }
    else
    {
        // This expression is slightly faster than max(0, log10Pow5(-e2) - 1).
        const uint32_t q = log10Pow5(-e2) - (int32_t)(-e2 > 1);
        e10 = (int32_t)q + e2;
        const auto i = (int32_t)(-e2 - q);
        const int32_t k = (int32_t)pow5bits(i) - BOOST_DECIMAL_POW5_BITCOUNT;
        const int32_t j = (int32_t)q - k;
        uint64_t pow5[4] {};
        generic_computePow5(i, pow5);
        vr = mulShift(4 * m2, pow5, j);
        vp = mulShift(4 * m2 + 2, pow5, j);
        vm = mulShift(4 * m2 - 1 - mmShift, pow5, j);

        #ifdef BOOST_DECIMAL_DEBUG
        printf("%s * 5^%d / 10^%d\n", s(mv), -e2, q);
        printf("%d %d %d %d\n", q, i, k, j);
        printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
        #endif

        if (q <= 1)
        {
            // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
            // mv = 4 m2, so it always has at least two trailing 0 bits.
            vrIsTrailingZeros = true;
            if (acceptBounds)
            {
                // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
                vmIsTrailingZeros = mmShift == 1;
            }
            else
            {
                // mp = mv + 2, so it always has at least one trailing 0 bit.
                --vp;
            }
        }
        else if (q < 127)
        {
            // We need to compute min(ntz(mv), pow5Factor(mv) - e2) >= q-1
            // <=> ntz(mv) >= q-1  &&  pow5Factor(mv) - e2 >= q-1
            // <=> ntz(mv) >= q-1    (e2 is negative and -e2 >= q)
            // <=> (mv & ((1 << (q-1)) - 1)) == 0
            // We also need to make sure that the left shift does not overflow.
            vrIsTrailingZeros = multipleOfPowerOf2(mv, q - 1);

            #ifdef BOOST_DECIMAL_DEBUG
            printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
            #endif
        }
    }

    #ifdef BOOST_DECIMAL_DEBUG
    printf("e10=%d\n", e10);
    printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
    printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
    #endif

    // Step 4: Find the shortest decimal representation in the interval of legal representations.
    uint32_t removed = 0;
    uint8_t lastRemovedDigit = 0;
    unsigned_128_type output {};

    while (vp / 10 > vm / 10)
    {
        vmIsTrailingZeros &= vm % 10 == 0;
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        lastRemovedDigit = (uint8_t) (vr % 10);
        vr /= 10;
        vp /= 10;
        vm /= 10;
        ++removed;
    }

    #ifdef BOOST_DECIMAL_DEBUG
    printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
    printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
    #endif

    if (vmIsTrailingZeros)
    {
        while (vm % 10 == 0)
        {
            vrIsTrailingZeros &= lastRemovedDigit == 0;
            lastRemovedDigit = (uint8_t) (vr % 10);
            vr /= 10;
            vp /= 10;
            vm /= 10;
            ++removed;
        }
    }

    #ifdef BOOST_DECIMAL_DEBUG
    printf("%s %d\n", s(vr), lastRemovedDigit);
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
    #endif

    if (vrIsTrailingZeros && (lastRemovedDigit == 5) && (vr % 2 == 0))
    {
        // Round even if the exact numbers is .....50..0.
        lastRemovedDigit = 4;
    }
    // We need to take vr+1 if vr is outside bounds, or we need to round up.
    output = vr + (unsigned_128_type)((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || (lastRemovedDigit >= 5));
    const auto exp = static_cast<std::int32_t>(e10 + removed);

    #ifdef BOOST_DECIMAL_DEBUG
    printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
    printf("O=%s\n", s(output));
    printf("EXP=%d\n", exp);
    #endif

    return {output, exp, ieeeSign};
}

template <typename T>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128(T val) noexcept;

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<float>(float f) noexcept
{
    static_assert(sizeof(float) == sizeof(uint32_t), "Float is not 32 bits");
    auto bits = bit_cast<uint32_t>(f);
    return generic_binary_to_decimal(bits, 23, 8, false);
}

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<double>(double d) noexcept
{
    static_assert(sizeof(double) == sizeof(uint64_t), "Float is not 64 bits");
    auto bits = bit_cast<uint64_t>(d);
    return generic_binary_to_decimal(bits, 52, 11, false);
}

#if BOOST_DECIMAL_LDBL_BITS == 64

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<long double>(long double ld) noexcept
{
    static_assert(sizeof(long double) == sizeof(uint64_t), "Float is not 64 bits");
    auto bits = bit_cast<uint64_t>(ld);
    return generic_binary_to_decimal(bits, 52, 11, false);
}

#elif BOOST_DECIMAL_LDBL_BITS == 80

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<long double>(long double ld) noexcept
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    auto bits = bit_cast<unsigned_128_type>(ld);
    #else
    auto trivial_bits = bit_cast<trivial_uint128>(ld);
    unsigned_128_type bits {trivial_bits};
    #endif

    #ifdef BOOST_DECIMAL_DEBUG
    // For some odd reason, this ends up with noise in the top 48 bits. We can
    // clear out those bits with the following line; this is not required, the
    // conversion routine should ignore those bits, but the debug output can be
    // confusing if they aren't 0s.
    bits &= (one << 80) - 1;
    #endif

    return generic_binary_to_decimal(bits, 64, 15, true);
}

#elif BOOST_DECIMAL_LDBL_BITS == 128

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<long double>(long double ld) noexcept
{
    auto bits = bit_cast<unsigned_128_type>(ld);

    #if LDBL_MANT_DIG == 113 // binary128 (e.g. ARM, S390X)
    return generic_binary_to_decimal(bits, 112, 15, true);
    #elif LDBL_MANT_DIG == 106 // ibm128 (e.g. PowerPC)
    return generic_binary_to_decimal(bits, 105, 11, true);
    #endif
}

#endif

#ifdef BOOST_DECIMAL_HAS_FLOAT128

/*
TODO(mborland): constexpr bit_cast does not work on these types

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<__float128>(__float128 d) noexcept
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    auto bits = bit_cast<unsigned_128_type>(d);
    #else
    auto trivial_bits = bit_cast<trivial_uint128>(d);
    unsigned_128_type bits {trivial_bits};
    #endif

    return generic_binary_to_decimal(bits, 112, 15, false);
}
*/
#endif

// Use float as an interchange format
#ifdef BOOST_DECIMAL_HAS_FLOAT16
template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<std::float16_t>(std::float16_t val) noexcept
{
    auto f = static_cast<float>(val);
    static_assert(sizeof(float) == sizeof(uint32_t), "Float is not 32 bits");
    auto bits = bit_cast<uint32_t>(f);
    return generic_binary_to_decimal(bits, 23, 8, false);
}
#endif

// Use float as an interchange format
#ifdef BOOST_DECIMAL_HAS_BFLOAT16
template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<std::bfloat16_t>(std::bfloat16_t val) noexcept
{
    float f = static_cast<float>(val);
    static_assert(sizeof(float) == sizeof(uint32_t), "Float is not 32 bits");
    auto bits = bit_cast<uint32_t>(f);
    return generic_binary_to_decimal(bits, 23, 8, false);
}
#endif

#ifdef BOOST_DECIMAL_HAS_FLOAT32
template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<std::float32_t>(std::float32_t f) noexcept
{
    static_assert(sizeof(std::float32_t) == sizeof(uint32_t), "Float is not 32 bits");
    auto bits = bit_cast<uint32_t>(f);
    return generic_binary_to_decimal(bits, 23, 8, false);
}
#endif

#ifdef BOOST_DECIMAL_HAS_FLOAT64
template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<std::float64_t>(std::float64_t f) noexcept
{
    static_assert(sizeof(std::float64_t) == sizeof(uint64_t), "std::float64_t is not 64 bits");
    auto bits = bit_cast<uint64_t>(f);
    return generic_binary_to_decimal(bits, 52, 11, false);
}
#endif

#ifdef BOOST_DECIMAL_HAS_STDFLOAT128

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR floating_decimal_128 floating_point_to_fd128<std::float128_t>(std::float128_t d) noexcept
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    auto bits = bit_cast<unsigned_128_type>(d);
    #else
    auto trivial_bits = bit_cast<trivial_uint128>(d);
    unsigned_128_type bits {trivial_bits};
    #endif

    return generic_binary_to_decimal(bits, 112, 15, false);
}

#endif

}}}} // Namespaces

#endif //BOOST_DECIMAL_RYU_GENERIC_128_HPP
