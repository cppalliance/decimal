// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_PROMOTION_HPP
#define BOOST_DECIMAL_DETAIL_PROMOTION_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <limits>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace impl {

// Assign explicit decimal values because the decimal32_fast size could be greater than that
// of decimal64 even though the precision is worse

template <typename T>
struct decimal_val
{
    static constexpr int value = 0;
};

template <>
struct decimal_val<decimal32>
{
    static constexpr int value = 32;
};

// Assign a higher value to the fast type for consistency of promotion
// Side effect is the same calculation will be faster with the same precision
template <>
struct decimal_val<decimal32_fast>
{
    static constexpr int value = 33;
};

template <>
struct decimal_val<decimal64>
{
    static constexpr int value = 64;
};

template <>
struct decimal_val<decimal128>
{
    static constexpr int value = 128;
};

template <typename T>
constexpr int decimal_val_v = decimal_val<T>::value;

// Promotes a single argument to double if it is an integer type
template<typename T>
struct promote_arg
{
    using type = std::conditional_t<detail::is_integral_v<T>, double, T>;
};

template<typename T>
using promote_arg_t = typename promote_arg<T>::type;

// Promote two arguments picking in order:
// 1) the highest precision decimal type
// 2) any decimal type
// 3) or the highest precision type in that order
template<typename T1, typename T2>
struct promote_2_args
{
    using type = std::conditional_t<(is_decimal_floating_point_v<T1> && is_decimal_floating_point_v<T2>),
                 std::conditional_t<(decimal_val_v<T1> > decimal_val_v<T2>), T1, T2>,
                 std::conditional_t<is_decimal_floating_point_v<T1>, T1,
                        std::conditional_t<is_decimal_floating_point_v<T2>, T2,
                                std::conditional_t<(sizeof(promote_arg_t<T1>) > sizeof(promote_arg_t<T2>)),
                                                    promote_arg_t<T1>, promote_arg_t<T2>>>>>;
};

template<typename T1, typename T2>
using promote_2_args_t = typename promote_2_args<T1, T2>::type;

} //namespace impl

// Promote N args using the rules of promote_2_args
template <typename... Args>
struct promote_args;

template <typename T>
struct promote_args<T>
{
    using type = impl::promote_arg_t<T>;
};

template <typename T, typename... Args>
struct promote_args<T, Args...>
{
    using type = impl::promote_2_args_t<impl::promote_arg_t<T>, typename promote_args<Args...>::type>;
};

template <typename... Args>
using promote_args_t = typename promote_args<Args...>::type;

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_PROMOTION_HPP
