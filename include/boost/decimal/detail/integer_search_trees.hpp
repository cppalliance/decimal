// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP
#define BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP

// https://stackoverflow.com/questions/1489830/efficient-way-to-determine-number-of-digits-in-an-integer?page=1&tab=scoredesc#tab-top
// https://graphics.stanford.edu/~seander/bithacks.html

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/emulated256.hpp>
#include <boost/decimal/detail/u128.hpp>
#include <boost/decimal/detail/u256.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstdint>
#include <limits>
#endif

namespace boost {
namespace decimal {
namespace detail {

// Generic solution
template <typename T>
constexpr auto num_digits(T x) noexcept -> int
{
    int digits = 0;

    while (x)
    {
        x /= 10;
        ++digits;
    }

    return digits;
}

template <>
constexpr auto num_digits(std::uint32_t x) noexcept -> int
{
    if (x >= UINT32_C(10000))
    {
        if (x >= UINT32_C(10000000))
        {
            if (x >= UINT32_C(100000000))
            {
                if (x >= UINT32_C(1000000000))
                {
                    return 10;
                }
                return 9;
            }
            return 8;
        }

        else if (x >= UINT32_C(100000))
        {
            if (x >= UINT32_C(1000000))
            {
                return 7;
            }
            return 6;
        }
        return 5;
    }
    else if (x >= UINT32_C(100))
    {
        if (x >= UINT32_C(1000))
        {
            return 4;
        }
        return 3;
    }
    else if (x >= UINT32_C(10))
    {
        return 2;
    }

    return 1;
}

template <>
constexpr auto num_digits(std::uint64_t x) noexcept -> int
{
    if (x >= UINT64_C(10000000000))
    {
        if (x >= UINT64_C(100000000000000))
        {
            if (x >= UINT64_C(10000000000000000))
            {
                if (x >= UINT64_C(100000000000000000)) 
                {
                    if (x >= UINT64_C(1000000000000000000))
                    {
                        if (x >= UINT64_C(10000000000000000000))
                        {
                            return 20;
                        }
                        return 19;
                    }
                    return 18;
                }
                return 17;
            }
            else if (x >= UINT64_C(1000000000000000))
            {
                return 16;
            }
            return 15;
        } 
        if (x >= UINT64_C(1000000000000))
        {
            if (x >= UINT64_C(10000000000000))
            {
                return 14;
            }
            return 13;
        }
        if (x >= UINT64_C(100000000000))
        {
            return 12;
        }
        return 11;
    }
    else if (x >= UINT64_C(100000))
    {
        if (x >= UINT64_C(10000000))
        {
            if (x >= UINT64_C(100000000))
            {
                if (x >= UINT64_C(1000000000))
                {
                    return 10;
                }
                return 9;
            }
            return 8;
        }
        if (x >= UINT64_C(1000000))
        {
            return 7;
        }
        return 6;
    }
    if (x >= UINT64_C(100))
    {
        if (x >= UINT64_C(1000))
        {
            if (x >= UINT64_C(10000))
            {
                return 5;
            }
            return 4;
        }
        return 3;
    }
    if (x >= UINT64_C(10))
    {
        return 2;
    }
    return 1;
}

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4307) // MSVC 14.1 warns of intergral constant overflow
#endif

constexpr int num_digits(const uint128& x) noexcept
{
    if (x.high == UINT64_C(0))
    {
        return num_digits(x.low);
    }

    // We start left at 19 because we already eliminated the high word being 0
    std::uint32_t left = 19U;
    std::uint32_t right = 38U;

    while (left < right)
    {
        std::uint32_t mid = (left + right + 1U) / 2U;

        if (x >= impl::emulated_128_pow10[mid])
        {
            left = mid;
        }
        else
        {
            right = mid - 1;
        }
    }

    return static_cast<int>(left + 1);
}

constexpr int num_digits(const uint256_t& x) noexcept
{
    if (x.high == 0)
    {
        return num_digits(x.low);
    }

    // 10^77
    auto current_power_of_10 {uint256_t{uint128{UINT64_C(15930919111324522770), UINT64_C(5327493063679123134)}, uint128{UINT64_C(12292710897160462336), UINT64_C(0)}}};

    for (int i = 78; i > 0; --i)
    {
        if (x >= current_power_of_10)
        {
            return i;
        }

        current_power_of_10 /= UINT64_C(10);
    }

    return 1;
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr auto num_digits(const uint128_t& x) noexcept -> int
{
    if (static_cast<std::uint64_t>(x >> 64) == UINT64_C(0))
    {
        return num_digits(static_cast<std::uint64_t>(x));
    }

    // We start left at 19 because we already eliminated the high word being 0
    std::uint32_t left = 19U;
    std::uint32_t right = 38U;

    while (left < right)
    {
        std::uint32_t mid = (left + right + 1U) / 2U;

        if (x >= impl::emulated_128_pow10[mid])
        {
            left = mid;
        }
        else
        {
            right = mid - 1;
        }
    }

    return static_cast<int>(left + 1);
}

#endif // Has int128

// Check benchmark_uints.cpp for timing
#if defined(__aarch64__) && defined(__APPLE__)

constexpr auto num_digits(const u128& x) noexcept -> int
{
    int msb {};
    if (x.high != 0)
    {
        msb = 64 + (64 - countl_zero(x.high));
    }
    else
    {
        msb = 64 - countl_zero(x.low);
    }

    const auto estimated_digits {(msb * 1000) / 3322 + 1};

    if (x >= impl::emulated_u128_pow10[estimated_digits])
    {
        return estimated_digits != 39 ? estimated_digits + 1 : estimated_digits;
    }
    else if (estimated_digits > 1 && x < impl::emulated_u128_pow10[estimated_digits - 1])
    {
        return estimated_digits - 1;
    }
    else
    {
        return estimated_digits;
    }
}

#else

constexpr int num_digits(const u128& x) noexcept
{
    int guess {};
    if (x.high != UINT64_C(0))
    {
        guess = num_digits(x.high) + 19;
    }
    else
    {
        return num_digits(x.low);
    }

    return x >= impl::emulated_u128_pow10[guess] && guess != 39 ? guess + 1 : guess;
}

#endif

/*
constexpr int num_digits(const u256& x) noexcept
{
    const auto digits = x.bytes;
    if (digits[2] == digits[3] == 0)
    {
        return num_digits(u128{digits[1], digits[0]});
    }

    int msb {};
    if (digits[3] != 0)
    {
        msb = 192 + (64 - countl_zero(digits[3]));
    }
    else if (digits[2] != 0)
    {
        msb = 128 + (64 - countl_zero(digits[2]));
    }

    const auto estimated_digits {(msb * 1000) / 3322 + 1};

    if (x >= impl::emulated_u256_pow10[estimated_digits])
    {
        return estimated_digits != 78 ? estimated_digits + 1 : estimated_digits;
    }
    else if (estimated_digits > 1 && x < impl::emulated_u256_pow10[estimated_digits - 1])
    {
        return estimated_digits - 1;
    }

    return estimated_digits;
}
*/

// Specializations with pruned branches for constructors
// Since we already have partial information we can greatly speed things up in this case
template <typename T>
constexpr auto d32_constructor_num_digits(T) noexcept -> std::enable_if_t<std::numeric_limits<T>::digits10 + 1 < 7, int>
{
    // Does not matter since it is guaranteed to fit
    return 0;
}

template <typename T>
constexpr auto d32_constructor_num_digits(T x) noexcept -> std::enable_if_t<(std::numeric_limits<T>::digits10 + 1 <= 10) &&
                                                                            (std::numeric_limits<T>::digits10 + 1 > 7), int>
{
    BOOST_DECIMAL_ASSERT(x >= 10000000);

    if (x >= 100000000)
    {
        if (x >= 1000000000)
        {
            return 10;
        }
        return 9;
    }
    return 8;
}

template <typename T>
constexpr auto d32_constructor_num_digits(T x) noexcept -> std::enable_if_t<(std::numeric_limits<T>::digits10 + 1 > 10) &&
                                                                            (std::numeric_limits<T>::digits10 + 1 <= 20), int>
{
    // We already know that x >= 10000000 (7 digits)
    BOOST_DECIMAL_ASSERT(x >= 10000000);

    if (x >= UINT64_C(10000000000))
    {
        if (x >= UINT64_C(100000000000000))
        {
            if (x >= UINT64_C(10000000000000000))
            {
                if (x >= UINT64_C(100000000000000000))
                {
                    if (x >= UINT64_C(1000000000000000000))
                    {
                        if (x >= UINT64_C(10000000000000000000))
                        {
                            return 20;
                        }
                        return 19;
                    }
                    return 18;
                }
                return 17;
            }
            else if (x >= UINT64_C(1000000000000000))
            {
                return 16;
            }
            return 15;
        }
        if (x >= UINT64_C(1000000000000))
        {
            if (x >= UINT64_C(10000000000000))
            {
                return 14;
            }
            return 13;
        }
        if (x >= UINT64_C(100000000000))
        {
            return 12;
        }
        return 11;
    }
    else // 10000000 <= x < 10000000000
    {
        if (x >= UINT64_C(100000000))
        {
            if (x >= UINT64_C(1000000000))
            {
                return 10;
            }
            return 9;
        }
        else // 10000000 <= x < 100000000
        {
            return 8;
        }
    }
}

template <typename T>
constexpr auto d32_constructor_num_digits(T x) noexcept -> std::enable_if_t<(std::numeric_limits<T>::digits10 + 1 > 20), int>
{
    // Anything bigger than uint64_t has no benefit so fall back to that
    return num_digits(x);
}

#ifdef BOOST_DECIMAL_HAS_INT128
constexpr auto d32_constructor_num_digits(uint128_t x) noexcept -> int
{
    return num_digits(x);
}
#endif

template <typename T>
constexpr auto d64_constructor_num_digits(T) noexcept -> std::enable_if_t<(std::numeric_limits<T>::digits10 + 1 <= 16), int>
{
    return 0;
}

template <typename T>
constexpr auto d64_constructor_num_digits(T x) noexcept -> std::enable_if_t<(std::numeric_limits<T>::digits10 + 1 > 16) &&
                                                                            (std::numeric_limits<T>::digits10 <= 20), int>
{
    // Pre-condition: x >= 10^16
    BOOST_DECIMAL_ASSERT(x >= UINT64_C(10000000000000000));

    if (x >= UINT64_C(100000000000000000))
    {
        if (x >= UINT64_C(1000000000000000000))
        {
            if (x >= UINT64_C(10000000000000000000))
            {
                return 20;
            }
            return 19;
        }
        return 18;
    }
    return 17;
}

template <typename T>
constexpr auto d64_constructor_num_digits(T x) noexcept -> std::enable_if_t<std::numeric_limits<T>::digits10 >= 20, int>
{
    return num_digits(x);
}

#ifdef BOOST_DECIMAL_HAS_INT128
constexpr auto d64_constructor_num_digits(uint128_t x) noexcept -> int
{
    return num_digits(x);
}
#endif

template <typename T>
constexpr auto d128_constructor_num_digits(T) noexcept -> std::enable_if_t<std::numeric_limits<T>::digits10 + 1 <= 34, int>
{
    return 0;
}

#ifdef BOOST_DECIMAL_HAS_INT128
constexpr auto d128_constructor_num_digits(uint128_t x) noexcept -> int
{
    // Pre-condition: we know x has at least 34 digits
    BOOST_DECIMAL_ASSERT(x >= detail::pow10(static_cast<uint128_t>(34)));

    constexpr auto digits35 {detail::pow10(static_cast<uint128_t>(35))};
    constexpr auto digits36 {detail::pow10(static_cast<uint128_t>(36))};
    constexpr auto digits37 {detail::pow10(static_cast<uint128_t>(37))};
    constexpr auto digits38 {detail::pow10(static_cast<uint128_t>(38))};
    constexpr auto digits39 {detail::pow10(static_cast<uint128_t>(39))};

    if (x >= digits38)
    {
        if (x >= digits39)
        {
            return 39;
        }
        return 38;
    }
    if (x >= digits36)
    {
        if (x >= digits37)
        {
            return 37;
        }
        return 36;
    }
    if (x >= digits35)
    {
        return 35;
    }
    return 34;  // Since we know x has at least 34 digits
}
#endif

constexpr auto d128_constructor_num_digits(uint128 x) noexcept -> int
{
    // Pre-condition: we know x has at least 34 digits
    BOOST_DECIMAL_ASSERT(x >= detail::pow10(static_cast<uint128>(34)));

    // Since we know that x has at least 34 digits we can get away with just comparing the high bits,
    // which reduces these to uint64_t comps instead of synthesized 128-bit

    constexpr auto digits35 {detail::pow10(static_cast<uint128>(35)).high};
    constexpr auto digits36 {detail::pow10(static_cast<uint128>(36)).high};
    constexpr auto digits37 {detail::pow10(static_cast<uint128>(37)).high};
    constexpr auto digits38 {detail::pow10(static_cast<uint128>(38)).high};
    constexpr auto digits39 {detail::pow10(static_cast<uint128>(39)).high};

    const auto x_high {x.high};

    if (x_high >= digits38)
    {
        if (x_high >= digits39)
        {
            return 39;
        }
        return 38;
    }
    if (x_high >= digits36)
    {
        if (x_high >= digits37)
        {
            return 37;
        }
        return 36;
    }
    if (x_high >= digits35)
    {
        return 35;
    }
    return 34;  // Since we know x has at least 34 digits
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP
