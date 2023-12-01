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
#include <boost/decimal/detail/cmath/frexp.hpp>
#include <boost/decimal/detail/cmath/ldexp.hpp>
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
                T{UINT64_C(8343058921465319890), -20},
                T{UINT64_C(3380972830755654137), -19, true},
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
                T{UINT64_C(1895325191056554968), -23}
            };

            result = z * (Y + tools::evaluate_polynomial(P, T(z * z)) / tools::evaluate_polynomial(Q, T(z * z)));
        }
    }
    else if (invert ? (z < 110) : (z < T{66, -1}))
    {
        //
        // We'll be calculating erfc:
        //
        invert = !invert;

        if (z < T{15, -1})
        {
            // Max Error found at long double precision =   3.239590e-20
            // Maximum Deviation Found:                     2.241e-20
            // Expected Error Term:                         -2.241e-20
            // Maximum Relative Change in Control Points:   5.110e-03
            constexpr T Y {UINT64_C(4059357643127441406), -19};
            constexpr std::array<T, 8> P = {
                T{UINT64_C(9809059221628120317), -20, true},
                T{UINT64_C(1599890899229691413), -19},
                T{UINT64_C(2223598216199357124), -19},
                T{UINT64_C(1273039217035773623), -19},
                T{UINT64_C(3840575303427624003), -20},
                T{UINT64_C(6284311608511567193), -21},
                T{UINT64_C(4412666545143917464), -22},
                T{UINT64_C(2666890683362956426), -26}
            };
            constexpr std::array<T, 7> Q = {
                T{UINT64_C(1), 0},
                T{UINT64_C(2032374749854694693), -18},
                T{UINT64_C(1783554549549694052), -18},
                T{UINT64_C(8679403262937605782), -19},
                T{UINT64_C(2480256069900216984), -19},
                T{UINT64_C(3966496318330022699), -20},
                T{UINT64_C(2792202373094490268), -21}
            };

            constexpr T half {5, -1};
            result = Y + tools::evaluate_polynomial(P, T(z - half)) / tools::evaluate_polynomial(Q, T(z - half));
        }
        else if (z < T{25, -1})
        {
            // Max Error found at long double precision =   3.686211e-21
            // Maximum Deviation Found:                     1.495e-21
            // Expected Error Term:                         -1.494e-21
            // Maximum Relative Change in Control Points:   1.793e-04

            constexpr T Y {UINT64_C(5067281723022460937), -19};
            constexpr std::array<T, 7> P = {
                T{UINT64_C(2435004762076984022), -20, true},
                T{UINT64_C(3435226879356714513), -20},
                T{UINT64_C(5054208243055449495), -20},
                T{UINT64_C(2574793259177573882), -20},
                T{UINT64_C(6693498441903543561), -21},
                T{UINT64_C(9080791441609952444), -22},
                T{UINT64_C(5159172666980500279), -23}
            };
            constexpr std::array<T, 7> Q = {
                T{UINT64_C(1), 0},
                T{UINT64_C(1716578616719303363), -18},
                T{UINT64_C(1264096348242803662), -18},
                T{UINT64_C(5123714378389690159), -19},
                T{UINT64_C(1209026230511209509), -19},
                T{UINT64_C(1580271978318874853), -20},
                T{UINT64_C(8978713707780316114), -22}
            };

            constexpr T one_and_half {15, -1};
            result = Y + tools::evaluate_polynomial(P, T(z - one_and_half)) / tools::evaluate_polynomial(Q, T(z - one_and_half));
        }
        else if (z < T{45, -1})
        {
            // Maximum Deviation Found:                     1.107e-20
            // Expected Error Term:                         -1.106e-20
            // Maximum Relative Change in Control Points:   1.709e-04
            // Max Error found at long double precision =   1.446908e-20
        }

        int expon {};
        auto hi {floor(ldexp(frexp(z, &expon), 32))};
        hi = ldexp(hi, expon - 32);
        auto lo {z - hi};
        auto sq {z * z};
        auto err_sqr {((hi * hi - sq) + 2 * hi * lo) + lo * lo};

        result *= exp(-sq) * exp(-err_sqr) / z;
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
