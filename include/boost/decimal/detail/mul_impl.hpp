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

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename ReturnType, typename T1, typename T2>
BOOST_DECIMAL_FORCE_INLINE constexpr auto mul_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                                   T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> ReturnType
{
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

    auto res_sig {static_cast<std::uint64_t>(lhs_sig) * static_cast<std::uint64_t>(rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

    // We don't need to use the regular binary search tree detail::num_digits(res_sig)
    // because we know that res_sig must be [1'000'000^2, 9'999'999^2] which only differ by one order
    // of magnitude in their number of digits
    const auto sig_dig {res_sig >= UINT64_C(10000000000000) ? 14 : 13};
    constexpr auto max_dig {std::numeric_limits<std::uint32_t>::digits10};
    res_sig /= detail::pow10(static_cast<std::uint64_t>(sig_dig - max_dig));
    res_exp += sig_dig - max_dig;

    const auto res_sig_32 {static_cast<typename ReturnType::sig_type>(res_sig)};

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

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
constexpr auto d64_mul_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                            T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> ReturnType
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
    constexpr auto max_dig {std::numeric_limits<std::uint64_t>::digits10};
    res_sig /= detail::pow10(static_cast<unsigned_int128_type>(sig_dig - max_dig));
    res_exp += sig_dig - max_dig;

    const auto res_sig_64 {static_cast<std::uint64_t>(res_sig)};

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

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_MUL_IMPL_HPP
