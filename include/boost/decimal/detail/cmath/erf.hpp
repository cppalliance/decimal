// Copyright John Maddock 2006.
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ERF_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ERF_HPP

#include <boost/decimal/detail/cmath/impl/kahan_sum.hpp>
#include <boost/decimal/detail/cmath/exp.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/numbers.hpp>
#include <limits>

namespace boost {
namespace decimal {

namespace detail {

//
// Asymptotic series for large z:
//
template <typename T>
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

template <typename T>
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

template <typename T>
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

}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ERF_HPP
