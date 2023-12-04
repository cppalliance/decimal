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

            constexpr T Y {UINT64_C(5405750274658203125), -19};
            constexpr std::array<T, 7> P = {
                T{UINT64_C(2952767165309728403), -21},
                T{UINT64_C(1418532458954956041), -20},
                T{UINT64_C(1049595846264322939), -20},
                T{UINT64_C(3439637959761000776), -21},
                T{UINT64_C(5906544119487763790), -22},
                T{UINT64_C(5234353806361740087), -23},
                T{UINT64_C(1898960430503312573), -24}
            };
            constexpr std::array<T, 7> Q = {
                T{UINT64_C(1), 0},
                T{UINT64_C(1193521601852856426), -18},
                T{UINT64_C(6032569643634543929), -19},
                T{UINT64_C(1654111424585405858), -19},
                T{UINT64_C(2597298709462031665), -20},
                T{UINT64_C(2216575682928936992), -21},
                T{UINT64_C(8041494641903097998), -23}
            };

            constexpr T three_and_half {35, -1};
            result = Y + tools::evaluate_polynomial(P, T(z - three_and_half)) / tools::evaluate_polynomial(Q, T(z - three_and_half));
        }
        else
        {
            // Max Error found at long double precision =   7.961166e-21
            // Maximum Deviation Found:                     6.677e-21
            // Expected Error Term:                         6.676e-21
            // Maximum Relative Change in Control Points:   2.319e-05

            constexpr T Y {UINT64_C(5582551956176757812), -19};

            constexpr std::array<T, 9> P = {
                T{UINT64_C(5934387930080502141), -21},
                T{UINT64_C(2806662310090897139), -20},
                T{UINT64_C(1415978352045830500), -19, true},
                T{UINT64_C(9780882011543005488), -19, true},
                T{UINT64_C(5473515277960120494), -18, true},
                T{UINT64_C(1386773046602453266), -17, true},
                T{UINT64_C(2712749487205398217), -17, true},
                T{UINT64_C(2925451527470094615), -17, true},
                T{UINT64_C(1688657744997996769), -17, true}
            };
            constexpr std::array<T, 9> Q = {
                T{UINT64_C(1), 0},
                T{UINT64_C(4729489111866453945), -18},
                T{UINT64_C(2367505431476957492), -17},
                T{UINT64_C(6000215173356931867), -17},
                T{UINT64_C(1317662516451495229), -16},
                T{UINT64_C(1781679249712834825), -16},
                T{UINT64_C(1824993905059152227), -16},
                T{UINT64_C(1043652514795785780), -16},
                T{UINT64_C(3083655118912242917), -17}
            };

            result = Y + tools::evaluate_polynomial(P, T(1 / z)) / tools::evaluate_polynomial(Q, T(1 / z));
        }

        int expon {};
        auto hi {floor(ldexp(frexp(z, &expon), 32))};
        hi = ldexp(hi, expon - 32);
        auto lo {z - hi};
        auto sq {z * z};
        auto err_sqr {((hi * hi - sq) + T{2, 0} * hi * lo) + lo * lo};

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
        result = T{1, 0} - result;
    }

    return result;
}

template <>
constexpr auto erf_impl<decimal128>(decimal128 z, bool invert) noexcept -> decimal128
{
    constexpr decimal128 zero {0, 0};
    constexpr decimal128 half {5, -1};

    if (z < zero)
    {
        if (!invert)
        {
            return -erf_impl(-z, invert);
        }
        else if (z < -half)
        {
            return 2 - erf_impl(-z, invert);
        }
        else
        {
            return 1 + erf_impl(-z, false);
        }
    }

    decimal128 result {};

    //
    // Big bunch of selection statements now to pick which
    // implementation to use, try to put most likely options
    // first:
    //
    if (z < half)
    {
        //
        // We're going to calculate erf:
        //
        if (z == zero)
        {
            result = zero;
        }
        else if (z < decimal128{1, -20})
        {
            constexpr decimal128 c {detail::uint128{UINT64_C(183185015307313), UINT64_C(4316214765445777362)}, -36};
            result = z * decimal128{UINT64_C(1125), -3} + z * c;
        }
        else
        {
            // Max Error found at long double precision =   2.342380e-35
            // Maximum Deviation Found:                     6.124e-36
            // Expected Error Term:                         -6.124e-36
            // Maximum Relative Change in Control Points:   3.492e-10
            constexpr decimal128 Y {UINT64_C(10841522216796875), -16};
            constexpr std::array<decimal128, 8> P = {
                decimal128{detail::uint128{UINT64_C(239754751511176), UINT64_C(15346977608939294094)}, -35},
                decimal128{detail::uint128{UINT64_C(192712955706190), UINT64_C(2786476198819993080)}, -34, true},
                decimal128{detail::uint128{UINT64_C(315600174339923), UINT64_C(3061015393610667132)}, -35, true},
                decimal128{detail::uint128{UINT64_C(61091917605891), UINT64_C(1019303663574361383)}, -35, true},
                decimal128{detail::uint128{UINT64_C(436787460032112), UINT64_C(1788731756814597798)}, -37, true},
                decimal128{detail::uint128{UINT64_C(306994537534154), UINT64_C(5857517254794866796)}, -38, true},
                decimal128{detail::uint128{UINT64_C(91970165438019), UINT64_C(5861580289485811316)}, -39, true},
                decimal128{detail::uint128{UINT64_C(186725770436288), UINT64_C(13306862545778890572)}, -41, true}
            };
            constexpr std::array<decimal128, 8> Q = {
                decimal128{UINT64_C(1)},
                decimal128{detail::uint128{UINT64_C(252912975277071), UINT64_C(16234303672316163784)}, -34},
                decimal128{detail::uint128{UINT64_C(54212866299291), UINT64_C(9947708872772716820)}, -34},
                decimal128{detail::uint128{UINT64_C(69574086016095), UINT64_C(17436381122513081906)}, -35},
                decimal128{detail::uint128{UINT64_C(58086374505287), UINT64_C(2736284848178772790)}, -36},
                decimal128{detail::uint128{UINT64_C(317762509029661), UINT64_C(14901341870138001204)}, -38},
                decimal128{detail::uint128{UINT64_C(106376826023067), UINT64_C(57314722672041808)}, -39},
                decimal128{detail::uint128{UINT64_C(169888257966113), UINT64_C(17571764770326690292)}, -41}
            };

            const auto z_squared {z * z};
            result = z * (Y + tools::evaluate_polynomial(P, z_squared) / tools::evaluate_polynomial(Q, z_squared));
        }
    }
    else if (invert ? (z < decimal128{110}) : (z < decimal128{UINT64_C(865), -2}))
    {
        //
        // We'll be calculating erfc:
        //
        invert = !invert;

        if (z < decimal128{1})
        {
            // Max Error found at long double precision =   3.246278e-35
            // Maximum Deviation Found:                     1.388e-35
            // Expected Error Term:                         1.387e-35
            // Maximum Relative Change in Control Points:   6.127e-05
            constexpr decimal128 Y {detail::uint128{UINT64_C(201595030518654), UINT64_C(473630177736155136)}, -34};
            constexpr std::array<decimal128, 10> P = {
                decimal128{detail::uint128{UINT64_C(347118283305744), UINT64_C(13376242280388530596)}, -35, true},
                decimal128{detail::uint128{UINT64_C(108837567018829), UINT64_C(8949668339020089396)}, -34},
                decimal128{detail::uint128{UINT64_C(205156638136972), UINT64_C(8479374702376111038)}, -34},
                decimal128{detail::uint128{UINT64_C(165456838044201), UINT64_C(8069456678105518694)}, -34},
                decimal128{detail::uint128{UINT64_C(79629242873361), UINT64_C(2204766815466333204)}, -34},
                decimal128{detail::uint128{UINT64_C(251989150980866), UINT64_C(8451275733071948234)}, -35},
                decimal128{detail::uint128{UINT64_C(535539364059100), UINT64_C(16183076954934542620)}, -36},
                decimal128{detail::uint128{UINT64_C(74543006006681), UINT64_C(16874855259041196514)}, -36},
                decimal128{detail::uint128{UINT64_C(62057810663588), UINT64_C(4225016817461922308)}, -37},
                decimal128{detail::uint128{UINT64_C(236651445527996), UINT64_C(10163568610288357464)}, -39}
            };
            constexpr std::array<decimal128, 11> Q = {
                decimal128{1},
                decimal128{detail::uint128{UINT64_C(134251975244461), UINT64_C(12266621785705425304)}, -33},
                decimal128{detail::uint128{UINT64_C(151087088804865), UINT64_C(7783954991533043640)}, -33},
                decimal128{detail::uint128{UINT64_C(101533324186242), UINT64_C(5983365784156864228)}, -33},
                decimal128{detail::uint128{UINT64_C(449605535730502), UINT64_C(10426028039653281378)}, -34},
                decimal128{detail::uint128{UINT64_C(136248852536558), UINT64_C(5020121607011525382)}, -34},
                decimal128{detail::uint128{UINT64_C(283036543896270), UINT64_C(9880778004342474900)}, -35},
                decimal128{detail::uint128{UINT64_C(389408639476240), UINT64_C(2248582422915465180)}, -36},
                decimal128{detail::uint128{UINT64_C(322701424296268), UINT64_C(7168735379570594832)}, -37},
                decimal128{detail::uint128{UINT64_C(123050804282587), UINT64_C(10903520715667482668)}, -38},
                decimal128{detail::uint128{UINT64_C(146728458516852), UINT64_C(13607062250089259428)}, -44}
            };

            result = Y + tools::evaluate_polynomial(P, z - half) / tools::evaluate_polynomial(Q, z - half);
        }
        else if (z < decimal128{15, -1})
        {
            // Max Error found at long double precision =   2.215785e-35
            // Maximum Deviation Found:                     1.539e-35
            // Expected Error Term:                         1.538e-35
            // Maximum Relative Change in Control Points:   6.104e-05
            constexpr decimal128 Y {detail::uint128{UINT64_C(247512601803296), UINT64_C(15871045498809073664)}, -34};
            constexpr std::array<decimal128, 10> P = {
                decimal128{detail::uint128{UINT64_C(157190807096733), UINT64_C(3137315625382477952)}, -35, true},
                decimal128{detail::uint128{UINT64_C(470641968793799), UINT64_C(4414359042974488606)}, -35},
                decimal128{detail::uint128{UINT64_C(91817523159857), UINT64_C(7399250419088684648)}, -34},
                decimal128{detail::uint128{UINT64_C(72372915581218), UINT64_C(10309284290091665052)}, -34},
                decimal128{detail::uint128{UINT64_C(334719143293246), UINT64_C(12410907560623277594)}, -35},
                decimal128{detail::uint128{UINT64_C(100623987889980), UINT64_C(3812727289885689320)}, -35},
                decimal128{detail::uint128{UINT64_C(201634177286597), UINT64_C(799217504105204558)}, -36},
                decimal128{detail::uint128{UINT64_C(262985005296582), UINT64_C(13926270613440862488)}, -37},
                decimal128{detail::uint128{UINT64_C(204098189489188), UINT64_C(7062163629122386192)}, -38},
                decimal128{detail::uint128{UINT64_C(72189464720907), UINT64_C(6671367611770889188)}, -39}
            };
            constexpr std::array<decimal128, 10> Q = {
                decimal128{1},
                decimal128{detail::uint128{UINT64_C(126293469034752), UINT64_C(6450544005567922118)}, -33},
                decimal128{detail::uint128{UINT64_C(133533437898934), UINT64_C(4515443098870771936)}, -33},
                decimal128{detail::uint128{UINT64_C(84192571838248), UINT64_C(9806577921514899802)}, -33},
                decimal128{detail::uint128{UINT64_C(349261792374621), UINT64_C(8697853943046476554)}, -34},
                decimal128{detail::uint128{UINT64_C(98992586623193), UINT64_C(17173790472898309662)}, -34},
                decimal128{detail::uint128{UINT64_C(191996836870529), UINT64_C(4384468307423291196)}, -35},
                decimal128{detail::uint128{UINT64_C(246146619886387), UINT64_C(5406971225063089448)}, -36},
                decimal128{detail::uint128{UINT64_C(189665960731903), UINT64_C(5272013552808781312)}, -37},
                decimal128{detail::uint128{UINT64_C(67084640707228), UINT64_C(2876771981794530406)}, -38}
            };
        }

        decimal128 hi {};
        decimal128 lo;
        int expon {};
        hi = floor(ldexp(frexp(z, &expon), 56));
        hi = ldexp(hi, expon - 56);
        lo = z - hi;
        auto sq = z * z;
        auto err_sqr = ((hi * hi - sq) + 2 * hi * lo) + lo * lo;
        result *= exp(-sq) * exp(-err_sqr) / z;
    }

    return result;
}

} //namespace detail

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
    const auto fp {fpclassify(z)};

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
