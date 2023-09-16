// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_APPLY_SIGN_HPP
#define BOOST_DECIMAL_DETAIL_APPLY_SIGN_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <limits>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4146)
#endif

namespace boost { namespace decimal { namespace detail {

template <typename Integer, typename Unsigned_Integer = detail::make_unsigned_t<Integer>,
          std::enable_if_t<std::numeric_limits<Integer>::is_signed, bool> = true>
constexpr Unsigned_Integer apply_sign(Integer val) noexcept
{
    return -(static_cast<Unsigned_Integer>(val));
}

template <typename Unsigned_Integer, std::enable_if_t<!std::numeric_limits<Unsigned_Integer>::is_signed, bool> = true>
constexpr Unsigned_Integer apply_sign(Unsigned_Integer val) noexcept
{
    return val;
}

template <typename Integer, typename Unsigned_Integer = detail::make_unsigned_t<Integer>,
        std::enable_if_t<std::numeric_limits<Integer>::is_signed, bool> = true>
constexpr Unsigned_Integer make_positive_unsigned(Integer val) noexcept
{
    return static_cast<Unsigned_Integer>(val < 0 ? apply_sign(val) : val);
}

template <typename Unsigned_Integer, std::enable_if_t<!std::numeric_limits<Unsigned_Integer>::is_signed, bool> = true>
constexpr Unsigned_Integer make_positive_unsigned(Unsigned_Integer val) noexcept
{
    return val;
}

template <typename Integer, std::enable_if_t<std::numeric_limits<Integer>::is_signed, bool> = true>
constexpr Integer make_signed_value(Integer val, bool sign) noexcept
{
    return sign ? -val : val;
}

template <typename Unsigned_Integer, typename Integer = detail::make_signed_t<Unsigned_Integer>,
          std::enable_if_t<!std::numeric_limits<Unsigned_Integer>::is_signed, bool> = true>
constexpr Integer make_signed_value(Unsigned_Integer val, bool sign) noexcept
{
    auto signed_val {static_cast<Integer>(val)};
    return sign ? -signed_val : signed_val;
}


} // namespace detail
} // namespace decimal
} // namespace boost

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif // BOOST_DECIMAL_DETAIL_APPLY_SIGN_HPP
