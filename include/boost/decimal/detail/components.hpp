// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_COMPONENTS_HPP
#define BOOST_DECIMAL_DETAIL_COMPONENTS_HPP

#include <boost/decimal/detail/config.hpp>

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

    significand_type sig;
    biased_exponent_type exp;
    bool sign;

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

}

using decimal32_components = impl::decimal_components<std::uint32_t, std::int32_t>;

using decimal32_fast_components = impl::decimal_components<std::uint_fast32_t, std::int_fast32_t>;

using decimal64_components = impl::decimal_components<std::uint64_t, std::int32_t>;

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_COMPONENTS_HPP
