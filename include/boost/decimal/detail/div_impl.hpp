// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_DIV_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_DIV_IMPL_HPP

#include <boost/decimal/detail/config.hpp>

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
    const auto big_sig_lhs {static_cast<std::uint64_t>(lhs.sig) * detail::pow10(detail::precision)};

    auto res_sig {big_sig_lhs / static_cast<std::uint64_t>(rhs.sig)};
    auto res_exp {(lhs.exp - detail::precision) - rhs.exp};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    if (res_sig == 0)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    return DecimalType{res_sig, res_exp, sign};
}

template <typename T>
constexpr auto d64_generic_div_impl(const T& lhs, const T& rhs, T& q) noexcept -> void
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    using unsigned_int128_type = boost::decimal::detail::uint128_t;
    #else
    using unsigned_int128_type = boost::decimal::detail::uint128;
    #endif

    bool sign {lhs.sign != rhs.sign};

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    constexpr auto tens_needed {detail::pow10(static_cast<unsigned_int128_type>(detail::precision_v<decimal64>))};
    const auto big_sig_lhs {static_cast<unsigned_int128_type>(lhs.sig) * tens_needed};

    auto res_sig {big_sig_lhs / static_cast<unsigned_int128_type>(rhs.sig)};
    auto res_exp {(lhs.exp - detail::precision_v<decimal64>) - rhs.exp};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > std::numeric_limits<std::uint64_t>::digits10)
    {
        res_sig /= static_cast<unsigned_int128_type>(detail::pow10(static_cast<std::uint64_t>(sig_dig - std::numeric_limits<std::uint64_t>::digits10)));
        res_exp += sig_dig - std::numeric_limits<std::uint64_t>::digits10;
    }

    const auto res_sig_64 {static_cast<std::uint64_t>(res_sig)};

    if (res_sig_64 == 0)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    q = T{res_sig_64, res_exp, sign};
}

template <typename T>
constexpr auto d128_generic_div_impl(T lhs, T rhs, T& q) noexcept -> void
{
    bool sign {lhs.sign != rhs.sign};

    const auto big_sig_lhs {detail::uint256_t(lhs.sig) * detail::uint256_t(pow10(detail::uint128(detail::precision_v<decimal128>)))};
    lhs.exp -= detail::precision_v<decimal128>;

    auto res_sig {big_sig_lhs / detail::uint256_t(rhs.sig)};
    auto res_exp {lhs.exp - rhs.exp};

    // TODO(mborland): Since the values are normalized is sig_dig always near 34?
    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > std::numeric_limits<detail::uint128>::digits10)
    {
        const auto digit_delta {sig_dig - std::numeric_limits<detail::uint128>::digits10};
        res_sig /= detail::uint256_t(pow10(detail::uint128(digit_delta)));
        res_exp += digit_delta;
    }

    if (res_sig == 0)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    q = T {res_sig.low, res_exp, sign};
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_DIV_IMPL_HPP
