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

namespace boost { namespace decimal { namespace detail {

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

static constexpr std::uint_fast8_t guess[] = {
    0 ,0 ,0 ,0 , 1 ,1 ,1 , 2 ,2 ,2 ,
    3 ,3 ,3 ,3 , 4 ,4 ,4 , 5 ,5 ,5 ,
    6 ,6 ,6 ,6 , 7 ,7 ,7 , 8 ,8 ,8 ,
    9 ,9 ,9 ,9 , 10,10,10, 11,11,11,
    12,12,12,12, 13,13,13, 14,14,14,
    15,15,15,15, 16,16,16, 17,17,17,
    18,18,18,18, 19
};

#ifdef BOOST_DECIMAL_HAS_INT128

// Assume that if someone is using 128 bit ints they are favoring the top end of the range
// Max value is 340,282,366,920,938,463,463,374,607,431,768,211,455 (39 digits)
constexpr auto num_digits(boost::decimal::detail::uint128_t x) noexcept -> int
{
    const auto high = static_cast<std::uint64_t>(x >> 64);

    if (high != 0)
    {
        const auto digits {guess[64 - countl_zero(high)]};
        return 19 + digits + (x >= impl::powers_of_10[digits]);
    }

    return num_digits(static_cast<std::uint64_t>(x));
}

#endif // Has int128

static constexpr uint128 correction_powers_of_10[] =
{
        uint128 {UINT64_C(0), UINT64_C(1)},
        uint128 {UINT64_C(0), UINT64_C(10)},
        uint128 {UINT64_C(0), UINT64_C(100)},
        uint128 {UINT64_C(0), UINT64_C(1000)},
        uint128 {UINT64_C(0), UINT64_C(10000)},
        uint128 {UINT64_C(0), UINT64_C(100000)},
        uint128 {UINT64_C(0), UINT64_C(1000000)},
        uint128 {UINT64_C(0), UINT64_C(10000000)},
        uint128 {UINT64_C(0), UINT64_C(100000000)},
        uint128 {UINT64_C(0), UINT64_C(1000000000)},
        uint128 {UINT64_C(0), UINT64_C(10000000000)},
        uint128 {UINT64_C(0), UINT64_C(100000000000)},
        uint128 {UINT64_C(0), UINT64_C(1000000000000)},
        uint128 {UINT64_C(0), UINT64_C(10000000000000)},
        uint128 {UINT64_C(0), UINT64_C(100000000000000)},
        uint128 {UINT64_C(0), UINT64_C(1000000000000000)},
        uint128 {UINT64_C(0), UINT64_C(10000000000000000)},
        uint128 {UINT64_C(0), UINT64_C(100000000000000000)},
        uint128 {UINT64_C(0), UINT64_C(1000000000000000000)},
        uint128 {UINT64_C(0), UINT64_C(10000000000000000000)},
        uint128 {UINT64_C(5), UINT64_C(7766279631452241920)},
        uint128 {UINT64_C(54), UINT64_C(3875820019684212736)},
        uint128 {UINT64_C(542), UINT64_C(1864712049423024128)},
        uint128 {UINT64_C(5421), UINT64_C(200376420520689664)},
        uint128 {UINT64_C(54210), UINT64_C(2003764205206896640)},
        uint128 {UINT64_C(542101), UINT64_C(1590897978359414784)},
        uint128 {UINT64_C(5421010), UINT64_C(15908979783594147840)},
        uint128 {UINT64_C(54210108), UINT64_C(11515845246265065472)},
        uint128 {UINT64_C(542101086), UINT64_C(4477988020393345024)},
        uint128 {UINT64_C(5421010862), UINT64_C(7886392056514347008)},
        uint128 {UINT64_C(54210108624), UINT64_C(5076944270305263616)},
        uint128 {UINT64_C(542101086242), UINT64_C(13875954555633532928)},
        uint128 {UINT64_C(5421010862427), UINT64_C(9632337040368467968)},
        uint128 {UINT64_C(54210108624275), UINT64_C(4089650035136921600)},
        uint128 {UINT64_C(542101086242752), UINT64_C(4003012203950112768)},
        uint128 {UINT64_C(5421010862427522), UINT64_C(3136633892082024448)},
        uint128 {UINT64_C(54210108624275221), UINT64_C(12919594847110692864)},
        uint128 {UINT64_C(542101086242752217), UINT64_C(68739955140067328)},
        uint128 {UINT64_C(5421010862427522170), UINT64_C(687399551400673280)},
        uint128 {UINT64_C(17316620476856118468), UINT64_C(6873995514006732800)},
};

static_assert(sizeof(correction_powers_of_10) == sizeof(uint128) * 40, "Should have 10^0 to 10^39");

constexpr auto num_digits(const uint128& x) noexcept -> int
{
    constexpr auto uint64_t_dig {std::numeric_limits<std::uint64_t>::digits10};
    constexpr auto uint64_t_bits {64};

    if (x.high != 0)
    {
        const auto digits {guess[uint64_t_bits - countl_zero(x.high)]};
        auto ret_val {uint64_t_dig + digits + (x >= correction_powers_of_10[digits])};

        // We need to make sure that deviations in the number of digits in the low word
        // (e.g. 18 or 20 decimal digits) are compensated for.
        ret_val += static_cast<int>(x > correction_powers_of_10[ret_val]);

        return ret_val;
    }

    const auto ret_val {num_digits(x.low)};
    return ret_val;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_INTEGER_SEARCH_TREES_HPP
