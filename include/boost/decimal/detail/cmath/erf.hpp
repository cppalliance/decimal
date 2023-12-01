// Copyright John Maddock 2006.
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ERF_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ERF_HPP

#include <boost/decimal/detail/cmath/impl/kahan_sum.hpp>
#include <boost/decimal/detail/cmath/impl/evaluate_polynomial.hpp>
#include <boost/decimal/detail/cmath/exp.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>
#include <limits>
#include <array>

namespace boost {
namespace decimal {

namespace detail {

//
// Asymptotic series for large z:
//
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
class erf_asympt_series_t
{
private:
    T result;
    T xx;
    int tk {1};

public:
    using result_type = T;

    constexpr erf_asympt_series_t(T z)
    {
        const auto neg_z_squared {-z * z};
        result = -exp(neg_z_squared) / sqrt(numbers::pi_v<T>);
        result /= z;
        xx = neg_z_squared * 2;
    }

    constexpr auto operator()() noexcept -> T
    {
        auto r {result};
        result *= tk / xx;
        tk += 2;
        if (fabs(r) < fabs(result))
        {
            result = 0;
        }

        return r;
    }

};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
class erf_series_near_zero
{
private:
    T term;
    T zz;
    int k {};

public:
    using result_type = T;

    explicit constexpr erf_series_near_zero(const T &z) : term {z}, zz {-z * z} {}

    constexpr auto operator()() noexcept -> T
    {
        auto result {term / (2 * k + 1)};
        term *= zz / ++k;
        return result;
    }
};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto erf_series_near_zero_sum(const T &x) noexcept -> T
{
    //
    // We need Kahan summation here, otherwise the errors grow fairly quickly.
    // This method is *much* faster than the alternatives even so.
    //
    constexpr T two_div_root_pi {2 / sqrt(numbers::pi_v<T>)};

    erf_series_near_zero<T> sum{x};
    return two_div_root_pi * tools::kahan_sum_series(sum, std::numeric_limits<T>::digits);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto erf_impl(T z, bool invert) noexcept -> T
{
    constexpr T zero {0, 0};

    if (z < zero)
    {
        if (!invert)
        {
            return -erf_impl(-z, invert);
        }
        else if (z < T{5, -1, true})
        {
            return 2 - erf_impl(-z, invert);
        }
        else
        {
            return 1 + erf_impl(-z, false);
        }
    }

    T result {};

    //
    // Big bunch of selection statements now to pick which
    // implementation to use, try to put most likely options
    // first:
    //
    if (z < T{5, -1})
    {
        //
        // We're going to calculate erf:
        //
        if (z == zero)
        {
            result = zero;
        }
        else if (z < T{1, -10})
        {
            constexpr T c {UINT64_C(3379167095512573896), -21};
            result = z * T{UINT64_C(1125), -3} + z * c;
        }
        else
        {
            // Max Error found at long double precision =   1.623299e-20
            // Maximum Deviation Found:                     4.326e-22
            // Expected Error Term:                         -4.326e-22
            // Maximum Relative Change in Control Points:   1.474e-04
            constexpr T Y {UINT64_C(1044948577880859375), -18};
            constexpr std::array<T, 6> P = {
                T{UINT64_C(8343058921465319889), -20},
                T{UINT64_C(3380972830755654136), -19, true},
                T{UINT64_C(5096027344060672046), -20, true},
                T{UINT64_C(9049063461585377944), -21, true},
                T{UINT64_C(4894686514647986692), -22, true},
                T{UINT64_C(2003056263661518778), -23, true}
            };
            constexpr std::array<T, 6> Q = {
                T{UINT64_C(1), 0},
                T{UINT64_C(4558173005158751724), -19},
                T{UINT64_C(9165373543562417920), -20},
                T{UINT64_C(1027226526759100312), -20},
                T{UINT64_C(6505117526878515487), -22},
                T{UINT64_C(1895325191056554967), -23}
            };

            result = z * (Y + tools::evaluate_polynomial(P, T(z * z)) / tools::evaluate_polynomial(Q, T(z * z)));
        }
    }
    else if (invert ? (z < 110) : (z < T{66, -1}))
    {
        //
        // We'll be calculating erfc:
        //
    }
    else
    {
        //
        // Any value of z larger than 110 will underflow to zero:
        //
        result = zero;
        invert = !invert;
    }

    if (invert)
    {
        result = 1 - result;
    }

    return result;
}

} //namespace detail

// TODO(mborland): Add special handling for decimal128

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto erf(T z) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    // Edge cases
    const auto fp {fpclassify(z)};

    if (fp == FP_ZERO || fp == FP_NAN)
    {
        return z;
    }
    else if (fp == FP_INFINITE)
    {
        return z < T{0} ? T{-1} : T{1};
    }

    return detail::erf_impl(z, false);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto erfc(T z) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    // Edge cases
    const auto fp {fpclassify((z))};

    if (fp == FP_NAN)
    {
        return z;
    }
    else if (fp == FP_INFINITE)
    {
        return z < T{0} ? T{2} : T{0};
    }

    return detail::erf_impl(z, true);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ERF_HPP
