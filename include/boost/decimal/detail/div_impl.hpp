// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_DIV_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_DIV_IMPL_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/u256.hpp>
#include "int128.hpp"

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename DecimalType, typename T>
BOOST_DECIMAL_FORCE_INLINE constexpr auto generic_div_impl(const T& lhs, const T& rhs) noexcept -> DecimalType
{
    bool sign {lhs.sign != rhs.sign};

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    constexpr auto ten_pow_precision {detail::pow10(static_cast<std::uint64_t>(detail::precision))};
    const auto big_sig_lhs {static_cast<std::uint64_t>(lhs.sig) * ten_pow_precision};

    auto res_sig {big_sig_lhs / static_cast<std::uint64_t>(rhs.sig)};
    auto res_exp {(lhs.exp - detail::precision) - rhs.exp};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    if (res_sig == 0U)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    return DecimalType{res_sig, res_exp, sign};
}

template <typename DecimalType, typename T>
constexpr auto d64_generic_div_impl(const T& lhs, const T& rhs) noexcept -> DecimalType
{
    using unsigned_int128_type = boost::int128::uint128_t;

    bool sign {lhs.sign != rhs.sign};

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    constexpr auto tens_needed {detail::pow10(static_cast<unsigned_int128_type>(detail::precision_v<decimal64_t>))};
    const auto big_sig_lhs {static_cast<unsigned_int128_type>(lhs.sig) * tens_needed};

    auto res_sig {big_sig_lhs / rhs.sig};
    auto res_exp {(lhs.exp - detail::precision_v<decimal64_t>) - rhs.exp};

    if (res_sig == 0U)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    return DecimalType{res_sig, res_exp, sign};
}

template <typename T>
constexpr auto d128_generic_div_impl(const T& lhs, const T& rhs, T& q) noexcept -> void
{
    bool sign {lhs.sign != rhs.sign};

    constexpr auto ten_pow_precision {pow10(int128::uint128_t(detail::precision_v<decimal128_t>))};
    const auto big_sig_lhs {detail::umul256(lhs.sig, ten_pow_precision)};

    auto res_sig {big_sig_lhs / rhs.sig};
    auto res_exp {lhs.exp - rhs.exp - detail::precision_v<decimal128_t>};

    if (res_sig[3] != 0 || res_sig[2] != 0)
    {
        const auto sig_dig {detail::num_digits(res_sig)};
        const auto digit_delta {sig_dig - std::numeric_limits<int128::uint128_t>::digits10};
        res_sig /= pow10(int128::uint128_t(digit_delta));
        res_exp += digit_delta;
    }
    else if (res_sig[1] == 0 && res_sig[0] == 0)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    BOOST_DECIMAL_ASSERT((res_sig[3] | res_sig[2]) == 0U);
    q = T {int128::uint128_t{res_sig[1], res_sig[0]}, res_exp, sign};
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_DIV_IMPL_HPP
