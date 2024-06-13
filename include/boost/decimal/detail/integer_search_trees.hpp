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

#if defined(__cpp_lib_array_constexpr) && __cpp_lib_array_constexpr >= 201603L

template <typename T, std::size_t N>
constexpr auto generate_array() noexcept -> std::array<T, N>
{
    std::array<T, N> values {};

    values[0] = T{1};
    for (std::size_t i {1}; i < N; ++i)
    {
        values[i] = values[i - 1] * UINT64_C(10);
    }

    return values;
}

#else

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

#endif // Constexpr array

#if defined(__cpp_lib_array_constexpr) && __cpp_lib_array_constexpr >= 201603L

constexpr int num_digits(const uint256_t& x) noexcept
{
    constexpr auto big_powers_of_10 = generate_array<boost::decimal::detail::uint256_t, 79>();

    if (x.high == UINT64_C(0) && x.low == UINT64_C(0))
    {
        return 1;
    }

    std::uint32_t left = 0U;
    std::uint32_t right = 78U;

    while (left < right)
    {
        std::uint32_t mid = (left + right + 1U) / 2U;

        if (x >= big_powers_of_10[mid])
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

#else

constexpr int num_digits(const uint256_t& x) noexcept
{
    if (x.high == 0)
    {
        return num_digits(x.low);
    }

    constexpr uint256_t max_digits = umul256({static_cast<uint128>(UINT64_C(10000000000000000000)) *
                                              static_cast<uint128>(UINT64_C(10000000000000000000))},
                                              {static_cast<uint128>(UINT64_C(10000000000000000000)) *
                                               static_cast<uint128>(UINT64_C(10000000000000000000))});

    uint256_t current_power_of_10 = max_digits;

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

#endif // Constexpr arrays

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#ifdef BOOST_DECIMAL_HAS_INT128

#if defined(__cpp_lib_array_constexpr) && __cpp_lib_array_constexpr >= 201603L

constexpr auto num_digits(boost::decimal::detail::uint128_t x) noexcept -> int
{
    constexpr auto big_powers_of_10 = generate_array<boost::decimal::detail::uint128_t, 39>();

    if (x == 0)
    {
        return 1;
    }

    std::uint32_t left = 0U;
    std::uint32_t right = 38U;

    while (left < right)
    {
        std::uint32_t mid = (left + right + 1U) / 2U;

        if (x >= big_powers_of_10[mid])
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

#else

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

#endif // constexpr arrays

#endif // Has int128

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP
