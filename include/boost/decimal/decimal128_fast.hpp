// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/add_impl.hpp>
#include <boost/decimal/detail/sub_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/div_impl.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <limits>
#include <cstdint>

#ifndef BOOST_DECIMAL_DECIMAL128_FAST_HPP
#define BOOST_DECIMAL_DECIMAL128_FAST_HPP

namespace boost {
namespace decimal {

namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_inf = std::numeric_limits<uint128>::max();
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_qnan = std::numeric_limits<uint128>::max() - 1;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_snan = std::numeric_limits<uint128>::max() - 2;

struct decimal128_fast_components
{
    using sig_type = uint128;

    uint128 sig;
    std::int32_t exp;
    bool sign;
};

} // namespace detail

class decimal128_fast final
{
public:
    using significand_type = detail::uint128;
    using exponent_type = std::uint_fast32_t;

private:
    // Instead of having to encode and decode at every operation
    // we store the constituent pieces directly

    significand_type significand_ {};
    exponent_type exponent_ {};
    bool sign_ {};

    constexpr auto isneg() const noexcept -> bool
    {
        return sign_;
    }

    constexpr auto full_significand() const noexcept -> significand_type
    {
        return significand_;
    }

    constexpr auto unbiased_exponent() const noexcept -> exponent_type
    {
        return exponent_;
    }

    constexpr auto biased_exponent() const noexcept -> std::int32_t
    {
        return static_cast<std::int32_t>(exponent_) - detail::bias_v<decimal128>;
    }

public:
    constexpr decimal128_fast() noexcept = default;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal128_fast(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal128_fast(Integer val) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_fast(Float val) noexcept;

    friend constexpr auto direct_init_d128(significand_type significand, exponent_type exponent, bool sign) noexcept -> decimal128_fast;

    // Classification functions
    friend constexpr auto signbit(decimal128_fast val) noexcept -> bool;
    friend constexpr auto isinf(decimal128_fast val) noexcept -> bool;
    friend constexpr auto isnan(decimal128_fast val) noexcept -> bool;
    friend constexpr auto issignaling(decimal128_fast val) noexcept -> bool;
    friend constexpr auto isnormal(decimal128_fast val) noexcept -> bool;

    // Comparison operators
    friend constexpr auto operator==(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator!=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator<(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator<=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator>=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    constexpr auto operator<=>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> std::partial_ordering;
    #endif

    // Unary arithmetic operators
    friend constexpr auto operator+(decimal128_fast rhs) noexcept -> decimal128_fast;
    friend constexpr auto operator-(decimal128_fast rhs) noexcept -> decimal128_fast;

    #if !defined(BOOST_DECIMAL_DISABLE_CLIB)

    // LCOV_EXCL_START
    // TODO(mborland): Fix with STL bindings and delete
    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, const decimal128_fast& d) -> std::basic_ostream<charT, traits>&
    {
        os << d.significand_ << "e";
        const auto biased_exp {d.biased_exponent()};
        if (biased_exp > 0)
        {
            os << '+';
        }
        os << biased_exp;

        return os;
    }
    // LCOV_EXCL_STOP

    #endif


    // LCOV_EXCL_STOP
};

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
#endif
constexpr decimal128_fast::decimal128_fast(T1 coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = detail::make_unsigned_t<T1>;

    const bool isneg {coeff < static_cast<T1>(0) || sign};
    sign_ = isneg;
    Unsigned_Integer unsigned_coeff {detail::make_positive_unsigned(coeff)};

    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision_v<decimal128>};

    // Strip digits
    if (unsigned_coeff_digits > detail::precision_v<decimal128> + 1)
    {
        const auto digits_to_remove {unsigned_coeff_digits - (detail::precision_v<decimal64> + 1)};

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic push
        #  pragma GCC diagnostic ignored "-Wconversion"
        #endif

        unsigned_coeff /= detail::pow10(static_cast<Unsigned_Integer>(digits_to_remove));

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic pop
        #endif

        exp += digits_to_remove;
        unsigned_coeff_digits -= digits_to_remove;
    }

    // Round as required
    if (reduced)
    {
        exp += static_cast<T2>(detail::fenv_round(unsigned_coeff, isneg));
    }

    significand_ = static_cast<significand_type>(unsigned_coeff);

    // Normalize the handling of zeros
    if (significand_ == detail::uint128{UINT64_C(0), UINT64_C(0)})
    {
        exp = 0;
    }

    const auto biased_exp {static_cast<exponent_type>(exp + detail::bias_v<decimal128>)};
    if (biased_exp > detail::max_biased_exp_v<decimal128>)
    {
        significand_ = detail::d128_fast_inf;
    }
    else
    {
        exponent_ = biased_exp;
    }
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
#endif
constexpr decimal128_fast::decimal128_fast(Integer val) noexcept
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal128_fast{static_cast<ConversionType>(val), 0, false};
}

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_REAL Float>
#else
template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
#endif
BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_fast::decimal128_fast(Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        significand_ = detail::d128_fast_qnan;
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        significand_ = detail::d128_fast_inf;
    }
    else
    #endif
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};
        *this = decimal128_fast {components.mantissa, components.exponent, components.sign};
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

constexpr auto direct_init_d128(decimal128_fast::significand_type significand, decimal128_fast::exponent_type exponent, bool sign) noexcept -> decimal128_fast
{
    decimal128_fast val {};
    val.significand_ = significand;
    val.exponent_ = exponent;
    val.sign_ = sign;

    return val;
}

constexpr auto signbit(decimal128_fast val) noexcept -> bool
{
    return val.sign_;
}

constexpr auto isinf(decimal128_fast val) noexcept -> bool
{
    return val.significand_ == detail::d128_fast_inf;
}

constexpr auto isnan(decimal128_fast val) noexcept -> bool
{
    return val.significand_ == detail::d128_fast_qnan ||
           val.significand_ == detail::d128_fast_snan;
}

constexpr auto issignaling(decimal128_fast val) noexcept -> bool
{
    return val.significand_ == detail::d128_fast_snan;
}

constexpr auto isnormal(decimal128_fast val) noexcept -> bool
{
    if (val.exponent_ <= static_cast<decimal128_fast::exponent_type>(detail::precision_v<decimal128> - 1))
    {
        return false;
    }

    return (val.significand_ != 0) && isfinite(val);
}

constexpr auto operator==(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return equal_parts_impl(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                            rhs.significand_, rhs.biased_exponent(), rhs.sign_);
}

constexpr auto operator!=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

constexpr auto operator<(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
#ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs) ||
        (!lhs.isneg() && rhs.isneg()))
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
    else if (isfinite(lhs) && isinf(rhs))
    {
        return !signbit(rhs);
    }
    else if (isinf(lhs) && isfinite(rhs))
    {
        return signbit(rhs);
    }
#else
    if (!lhs.isneg() && rhs.isneg())
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
#endif

    return less_parts_impl(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                           rhs.significand_, rhs.biased_exponent(), rhs.sign_);
}

constexpr auto operator<=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

constexpr auto operator>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    return rhs < lhs;
}

constexpr auto operator>=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !(lhs < rhs);
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

constexpr auto operator<=>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> std::partial_ordering
{
    if (lhs < rhs)
    {
        return std::partial_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::partial_ordering::greater;
    }
    else if (lhs == rhs)
    {
        return std::partial_ordering::equivalent;
    }

    return std::partial_ordering::unordered;
}

#endif

constexpr auto operator+(decimal128_fast rhs) noexcept -> decimal128_fast
{
    return rhs;
}

constexpr auto operator-(decimal128_fast rhs) noexcept -> decimal128_fast
{
    rhs.sign_ = !rhs.sign_;
    return rhs;
}

} // namespace decimal
} // namespace boost

namespace std {

BOOST_DECIMAL_EXPORT template<>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128_fast>
#else
struct numeric_limits<boost::decimal::decimal128_fast>
#endif
{

#ifdef _MSC_VER
    public:
#endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_specialized = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_signed = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_integer = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_exact = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_infinity = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_quiet_NaN = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_signaling_NaN = true;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_denorm_loss = true;
    #endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_round_style round_style = std::round_indeterminate;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_iec559 = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_bounded = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_modulo = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  digits = 34;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  radix = 10;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  min_exponent = -6142;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  min_exponent10 = min_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_exponent = 6145;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_exponent10 = max_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool traps = numeric_limits<std::uint64_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = true;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (min)        () -> boost::decimal::decimal128_fast { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (max)        () -> boost::decimal::decimal128_fast { return {boost::decimal::detail::uint128{UINT64_C(999'999'999'999'999), UINT64_C(9'999'999'999'999'999'999)}, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto lowest       () -> boost::decimal::decimal128_fast { return {boost::decimal::detail::uint128{UINT64_C(999'999'999'999'999), UINT64_C(9'999'999'999'999'999'999)}, max_exponent, true}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto epsilon      () -> boost::decimal::decimal128_fast { return {1, -34}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto round_error  () -> boost::decimal::decimal128_fast { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto infinity     () -> boost::decimal::decimal128_fast { return boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_inf, 0, false); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto quiet_NaN    () -> boost::decimal::decimal128_fast { return boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_qnan, 0, false); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto signaling_NaN() -> boost::decimal::decimal128_fast { return boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_snan, 0, false); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto denorm_min   () -> boost::decimal::decimal128_fast { return {1, boost::decimal::detail::etiny_v<boost::decimal::decimal128>}; }
};

}

#endif //BOOST_DECIMAL_DECIMAL128_FAST_HPP
