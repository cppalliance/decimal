// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_COMPONENTS_HPP
#define BOOST_DECIMAL_DETAIL_COMPONENTS_HPP

#include <boost/decimal/detail/config.hpp>
#include "int128.hpp"

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace impl {

template <typename SigType, typename BiasedExpType>
struct decimal_components
{
    using significand_type = SigType;
    using biased_exponent_type = BiasedExpType;

    significand_type sig {};
    biased_exponent_type exp {};
    bool sign {};

    constexpr decimal_components() = default;
    constexpr decimal_components(const decimal_components& rhs) = default;
    constexpr decimal_components& operator=(const decimal_components& rhs) = default;
    constexpr decimal_components(SigType sig_, BiasedExpType exp_, bool sign_) : sig{sig_}, exp{exp_}, sign{sign_} {}

    constexpr auto full_significand() const -> significand_type
    {
        return sig;
    }

    constexpr auto biased_exponent() const -> biased_exponent_type
    {
        return exp;
    }

    constexpr auto isneg() const -> bool
    {
        return sign;
    }
};

} // namespace impl

using decimal32_t_components = impl::decimal_components<std::uint32_t, std::int32_t>;

using decimal_fast32_t_components = impl::decimal_components<std::uint_fast32_t, std::int_fast32_t>;

using decimal64_t_components = impl::decimal_components<std::uint64_t, std::int32_t>;

using decimal128_t_components = impl::decimal_components<boost::int128::uint128_t, std::int32_t>;

using decimal_fast128_t_components = impl::decimal_components<boost::int128::uint128_t, std::int_fast32_t>;

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_COMPONENTS_HPP
