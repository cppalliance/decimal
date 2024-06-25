// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_MUL_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_MUL_IMPL_HPP

#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/emulated256.hpp>
#include <boost/decimal/detail/power_tables.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

// Each type has two different multiplication impls
// 1) Returns a decimal type and lets the constructor handle with shrinking the significand
// 2) Returns a struct of the constituent components (used with FMAs)

template <typename ReturnType, typename T, typename U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto mul_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                   T rhs_sig, U rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<std::is_same<ReturnType, decimal32_fast>::value, ReturnType>
{
    using mul_type = std::uint_fast64_t;

    const auto res_sig {static_cast<mul_type>(lhs_sig) * static_cast<mul_type>(rhs_sig)};
    const auto res_exp {lhs_exp + rhs_exp};

    return {res_sig, res_exp, lhs_sign != rhs_sign && res_sig != static_cast<mul_type>(0)};
}

template <typename ReturnType, typename T, typename U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto mul_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                   T rhs_sig, U rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<std::is_same<ReturnType, decimal32>::value, ReturnType>
{
    using mul_type = std::uint_fast64_t;

    // The constructor needs to calculate the number of digits in the significand which for uint128 is slow
    // Since we know the value of res_sig is constrained to [1'000'000^2, 9'999'999^2] which equates to
    // either 13 or 14 decimal digits we can use a single division to make binary search occur with
    // uint32_t instead. 14 - 5 = 9 or 13 - 5 = 8 which are both still greater than or equal to
    // digits10 + 1 for rounding which is 8 decimal digits

    auto res_sig {(static_cast<mul_type>(lhs_sig) * static_cast<mul_type>(rhs_sig)) / pow10(static_cast<mul_type>(5))};
    auto res_exp {lhs_exp + rhs_exp + static_cast<U>(5)};

    return {static_cast<std::uint32_t>(res_sig), res_exp, lhs_sign != rhs_sign && res_sig != static_cast<mul_type>(0)};
}

template <typename ReturnType, typename T, typename U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto mul_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                   T rhs_sig, U rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<!detail::is_decimal_floating_point_v<ReturnType>, ReturnType>
{
    using mul_type = std::uint_fast64_t;

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs;
    #endif

    bool sign {lhs_sign != rhs_sign};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents
    //
    // We use a 64 bit resultant significand because the two 23-bit unsigned significands will always fit

    auto res_sig {static_cast<mul_type>(lhs_sig) * static_cast<mul_type>(rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

    // We don't need to use the regular binary search tree detail::num_digits(res_sig)
    // because we know that res_sig must be [1'000'000^2, 9'999'999^2] which only differ by one order
    // of magnitude in their number of digits
    const auto sig_dig {res_sig >= UINT64_C(10000000000000) ? 14 : 13};
    constexpr auto max_dig {std::numeric_limits<typename ReturnType::significand_type>::digits10};
    res_sig /= detail::pow10(static_cast<mul_type>(sig_dig - max_dig));
    res_exp += sig_dig - max_dig;

    const auto res_sig_32 {static_cast<typename ReturnType::significand_type>(res_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    if (res_sig_32 == 0)
    {
        sign = false;
    }

    return {res_sig_32, res_exp, sign};
}

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto d64_mul_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                       T rhs_sig, U rhs_exp, bool rhs_sign) noexcept
                                                       -> std::enable_if_t<detail::is_decimal_floating_point_v<ReturnType>, ReturnType>
{
    // Clang 6-12 yields incorrect results with builtin u128, so we force usage of our version
    #if defined(BOOST_DECIMAL_HAS_INT128) && (!defined(__clang_major__) || (__clang_major__) > 12)
    using unsigned_int128_type = boost::decimal::detail::uint128_t;
    #else
    using unsigned_int128_type = boost::decimal::detail::uint128;
    #endif

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents
    //
    // The constructor needs to calculate the number of digits in the significand which for uint128 is slow
    // Since we know the value of res_sig is constrained to [(10^16)^2, (10^17 - 1)^2] which equates to
    // either 31 or 32 decimal digits we can use a single division to make binary search occur with
    // uint_fast64_t instead. 32 - 13 = 19 or 31 - 13 = 18 which are both still greater than
    // digits10 + 1 for rounding which is 17 decimal digits

    auto res_sig {(static_cast<unsigned_int128_type>(lhs_sig) * static_cast<unsigned_int128_type>(rhs_sig)) / pow10(static_cast<unsigned_int128_type>(13))};
    auto res_exp {lhs_exp + rhs_exp + static_cast<U>(13)};

    return {static_cast<std::uint64_t>(res_sig), res_exp, lhs_sign != rhs_sign && res_sig != 0};
}

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto d64_mul_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                       T rhs_sig, U rhs_exp, bool rhs_sign) noexcept
                                                       -> std::enable_if_t<!detail::is_decimal_floating_point_v<ReturnType>, ReturnType>
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    using unsigned_int128_type = boost::decimal::detail::uint128_t;
    constexpr auto comp_value {impl::builtin_128_pow10[31]};
    #else
    using unsigned_int128_type = boost::decimal::detail::uint128;
    constexpr auto comp_value {impl::emulated_128_pow10[31]};
    #endif

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs;
    #endif

    bool sign {lhs_sign != rhs_sign};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents

    auto res_sig {static_cast<unsigned_int128_type>(lhs_sig) * static_cast<unsigned_int128_type>(rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

    const auto sig_dig {res_sig >= comp_value ? 32 : 31};
    constexpr auto max_dig {std::numeric_limits<typename ReturnType::significand_type>::digits10};
    res_sig /= detail::pow10(static_cast<unsigned_int128_type>(sig_dig - max_dig));
    res_exp += sig_dig - max_dig;

    const auto res_sig_64 {static_cast<typename ReturnType::significand_type>(res_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_64
              << "\nres exp: " << res_exp << std::endl;
    #endif

    // Always return positive zero
    if (res_sig_64 == 0)
    {
        sign = false;
    }

    return {res_sig_64, res_exp, sign};
}

template <typename ReturnType, typename T1, typename T2>
constexpr auto d128_mul_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                             T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> ReturnType
{
    bool sign {lhs_sign != rhs_sign};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents
    auto res_sig {detail::umul256(lhs_sig, rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

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

    BOOST_DECIMAL_ASSERT(res_sig.high == uint128(0,0));
    return {res_sig.low, res_exp, sign};
}

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL U1,
                               BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto d128_fast_mul_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                                  T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> ReturnType
{
    bool sign {lhs_sign != rhs_sign};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents
    auto res_sig {detail::umul256(lhs_sig, rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

    constexpr auto comp_value {impl::emulated_256_pow10[67]};
    const auto sig_dig {res_sig >= comp_value ? 68 : 67};

    constexpr auto max_dig {std::numeric_limits<typename ReturnType::significand_type>::digits10};
    res_sig /= detail::pow10(static_cast<uint256_t>(sig_dig - max_dig));
    res_exp += sig_dig - max_dig;

    if (res_sig == 0)
    {
        sign = false;
    }

    BOOST_DECIMAL_ASSERT(res_sig.high == uint128(0,0));
    return {res_sig.low, res_exp, sign};
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_MUL_IMPL_HPP
