// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL64_FAST_HPP
#define BOOST_DECIMAL_DECIMAL64_FAST_HPP

#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/add_impl.hpp>
#include <boost/decimal/detail/sub_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/div_impl.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <limits>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_fast_inf = std::numeric_limits<std::uint_fast64_t>::max();
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_fast_qnan = std::numeric_limits<std::uint_fast64_t>::max() - 1;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_fast_snan = std::numeric_limits<std::uint_fast64_t>::max() - 2;

struct decimal64_fast_components
{
    using sig_type = std::uint_fast64_t;

    std::uint_fast64_t sig;
    std::int32_t exp;
    bool sign;
};

} // namespace detail

class decimal64_fast final
{
public:
    using significand_type = std::uint_fast64_t;
    using exponent_type = std::uint_fast16_t;

private:
    // In regular decimal64 we have to decode the significand end exponent
    // Here we will store them directly to avoid the overhead of decoding

    std::uint_fast64_t significand_ {};
    std::uint_fast16_t exponent_ {};
    bool sign_ {};

    constexpr auto isneg() const noexcept -> bool
    {
        return sign_;
    }

    constexpr auto full_significand() const noexcept -> std::uint_fast64_t
    {
        return significand_;
    }

    constexpr auto unbiased_exponent() const noexcept -> std::uint_fast16_t
    {
        return exponent_;
    }

    constexpr auto biased_exponent() const noexcept -> std::int32_t
    {
        return static_cast<std::int32_t>(exponent_) - detail::bias_v<decimal64>;
    }

public:
    constexpr decimal64_fast() noexcept = default;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal64_fast(T1 coeff, T2 exp, bool sign = false) noexcept;
};

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal64_fast::decimal64_fast(T1 coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = detail::make_unsigned_t<T1>;

    const bool isneg {coeff < static_cast<T1>(0) || sign};
    sign_ = isneg;
    Unsigned_Integer unsigned_coeff {detail::make_positive_unsigned(coeff)};

    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision_v<decimal64>};

    // Strip digits and round as required
    if (reduced)
    {
        const auto digits_to_remove {static_cast<Unsigned_Integer>(unsigned_coeff_digits - (detail::precision_v<decimal64> + 1))};

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic push
        #  pragma GCC diagnostic ignored "-Wconversion"
        #endif

        unsigned_coeff /= static_cast<Unsigned_Integer>(detail::pow10(digits_to_remove));

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic pop
        #endif

        exp += static_cast<exponent_type>(digits_to_remove);
        exp += static_cast<T2>(detail::fenv_round(unsigned_coeff, isneg));
    }

    significand_ = static_cast<significand_type>(unsigned_coeff);

    // Normalize the handling of zeros
    if (significand_ == UINT32_C(0))
    {
        exp = 0;
    }

    auto biased_exp {static_cast<std::uint_fast32_t>(exp + detail::bias)};
    if (biased_exp > std::numeric_limits<std::uint8_t>::max())
    {
        significand_ = detail::d64_fast_inf;
    }
    else
    {
        exponent_ = static_cast<exponent_type>(biased_exp);
    }
}

} // namespace decimal
} // namespace boost

namespace std {

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64_fast>;
#else
struct numeric_limits<boost::decimal::decimal64_fast>
#endif
{

};

} // namespace std

#endif //BOOST_DECIMAL_DECIMAL64_FAST_HPP
