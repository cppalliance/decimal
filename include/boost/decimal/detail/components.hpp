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

struct decimal32_components
{
    using significand_type = std::uint32_t;
    using biased_exponent_type = std::int32_t;

    significand_type sig;
    biased_exponent_type exp;
    bool sign;
};

struct decimal32_fast_components
{
    using significand_type = std::uint32_t;
    using biased_exponent_type = std::int32_t;

    significand_type sig;
    biased_exponent_type exp;
    bool sign;
};

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_COMPONENTS_HPP
