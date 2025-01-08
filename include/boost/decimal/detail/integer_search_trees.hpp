// Copyright 2023 Matt Borland
// Copyright 2021 Daniel Lemire
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP
#define BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP

// https://stackoverflow.com/questions/1489830/efficient-way-to-determine-number-of-digits-in-an-integer?page=1&tab=scoredesc#tab-top
// https://graphics.stanford.edu/~seander/bithacks.html
// https://lemire.me/blog/2021/06/03/computing-the-number-of-digits-of-an-integer-even-faster/

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/emulated256.hpp>
#include <boost/decimal/detail/countl.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstdint>
#include <limits>
#endif

namespace boost {
namespace decimal {
namespace detail {

constexpr auto int_log2(std::uint32_t x) noexcept -> int
{
    return 31 - unchecked_countl_zero(x | 1);
}

constexpr auto int_log2(std::uint64_t x) noexcept -> int
{
    return 63 - unchecked_countl_zero(x | 1);
}

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

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t uint32_t_zeros_table[] = {
        4294967296,  8589934582,  8589934582,  8589934582,  12884901788,
        12884901788, 12884901788, 17179868184, 17179868184, 17179868184,
        21474826480, 21474826480, 21474826480, 21474826480, 25769703776,
        25769703776, 25769703776, 30063771072, 30063771072, 30063771072,
        34349738368, 34349738368, 34349738368, 34349738368, 38554705664,
        38554705664, 38554705664, 41949672960, 41949672960, 41949672960,
        42949672960, 42949672960
};

template <>
constexpr auto num_digits(std::uint32_t x) noexcept -> int
{
    return static_cast<int>((x + uint32_t_zeros_table[int_log2(x)]) >> 32U);
}

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t uint64_t_zeros_table[] = {
            9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999,
            9999999999, 99999999999, 999999999999, 9999999999999, 99999999999999,
            999999999999999ULL, 9999999999999999ULL, 99999999999999999ULL,
            999999999999999999ULL, 9999999999999999999ULL
};

template <>
constexpr auto num_digits(std::uint64_t x) noexcept -> int
{
    auto y = static_cast<int>(19 * int_log2(x) >> 6);
    y += static_cast<int>(x > uint64_t_zeros_table[y]);
    return y + 1;
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

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP
