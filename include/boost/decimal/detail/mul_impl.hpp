// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_MUL_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_MUL_IMPL_HPP

#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename ReturnType, typename T1, typename T2>
constexpr auto mul_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
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

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > 9)
    {
        res_sig /= detail::pow10(static_cast<std::uint64_t>(sig_dig - 9));
        res_exp += sig_dig - 9;
    }

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

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_MUL_IMPL_HPP
