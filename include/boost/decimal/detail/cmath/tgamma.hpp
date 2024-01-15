// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T,
          const std::size_t CoefsSize>
std::array<T, CoefsSize> coefs_array_table = {{ T { } }};

template <>
std::array<::boost::decimal::decimal32, static_cast<std::size_t>(UINT8_C(13))> coefs_array_table<::boost::decimal::decimal32, static_cast<std::size_t>(UINT8_C(13))> =
{{
     ::boost::decimal::decimal32 { UINT64_C(577215664901532861), -18 -  0 }, // * z^2
    -::boost::decimal::decimal32 { UINT64_C(655878071520253881), -18 -  0 }, // * z^3
    -::boost::decimal::decimal32 { UINT64_C(420026350340952355), -18 -  1 }, // * z^4
    +::boost::decimal::decimal32 { UINT64_C(166538611382291490), -18 -  0 }, // * z^5
    -::boost::decimal::decimal32 { UINT64_C(421977345555443367), -18 -  1 }, // * z^6
    -::boost::decimal::decimal32 { UINT64_C(962197152787697356), -18 -  2 }, // * z^7
    +::boost::decimal::decimal32 { UINT64_C(721894324666309954), -18 -  2 }, // * z^8
    -::boost::decimal::decimal32 { UINT64_C(116516759185906511), -18 -  2 }, // * z^9
    -::boost::decimal::decimal32 { UINT64_C(215241674114950973), -18 -  3 }, // * z^10
    +::boost::decimal::decimal32 { UINT64_C(128050282388116186), -18 -  3 }, // * z^11
    -::boost::decimal::decimal32 { UINT64_C(201348547807882387), -18 -  4 }, // * z^12
    -::boost::decimal::decimal32 { UINT64_C(125049348214267066), -18 -  5 }, // * z^13
    +::boost::decimal::decimal32 { UINT64_C(113302723198169588), -18 -  5 }, // * z^14
}};

template <>
std::array<::boost::decimal::decimal64, static_cast<std::size_t>(UINT8_C(25))> coefs_array_table<::boost::decimal::decimal64, static_cast<std::size_t>(UINT8_C(25))> =
{{
     ::boost::decimal::decimal64 { UINT64_C(577215664901532861), -18 -  0 }, // * z^2
    -::boost::decimal::decimal64 { UINT64_C(655878071520253881), -18 -  0 }, // * z^3
    -::boost::decimal::decimal64 { UINT64_C(420026350340952355), -18 -  1 }, // * z^4
    +::boost::decimal::decimal64 { UINT64_C(166538611382291490), -18 -  0 }, // * z^5
    -::boost::decimal::decimal64 { UINT64_C(421977345555443367), -18 -  1 }, // * z^6
    -::boost::decimal::decimal64 { UINT64_C(962197152787697356), -18 -  2 }, // * z^7
    +::boost::decimal::decimal64 { UINT64_C(721894324666309954), -18 -  2 }, // * z^8
    -::boost::decimal::decimal64 { UINT64_C(116516759185906511), -18 -  2 }, // * z^9
    -::boost::decimal::decimal64 { UINT64_C(215241674114950973), -18 -  3 }, // * z^10
    +::boost::decimal::decimal64 { UINT64_C(128050282388116186), -18 -  3 }, // * z^11
    -::boost::decimal::decimal64 { UINT64_C(201348547807882387), -18 -  4 }, // * z^12
    -::boost::decimal::decimal64 { UINT64_C(125049348214267066), -18 -  5 }, // * z^13
    +::boost::decimal::decimal64 { UINT64_C(113302723198169588), -18 -  5 }, // * z^14
    -::boost::decimal::decimal64 { UINT64_C(205633841697760710), -18 -  6 }, // * z^15
    +::boost::decimal::decimal64 { UINT64_C(611609510448141582), -18 -  8 }, // * z^16
    +::boost::decimal::decimal64 { UINT64_C(500200764446922293), -18 -  8 }, // * z^17
    -::boost::decimal::decimal64 { UINT64_C(118127457048702014), -18 -  8 }, // * z^18
    +::boost::decimal::decimal64 { UINT64_C(104342671169110051), -18 -  9 }, // * z^19
    +::boost::decimal::decimal64 { UINT64_C(778226343990507125), -18 - 11 }, // * z^20
    -::boost::decimal::decimal64 { UINT64_C(369680561864220571), -18 - 11 }, // * z^21
    +::boost::decimal::decimal64 { UINT64_C(510037028745447598), -18 - 12 }, // * z^22
    -::boost::decimal::decimal64 { UINT64_C(205832605356650678), -18 - 13 }, // * z^23
    -::boost::decimal::decimal64 { UINT64_C(534812253942301798), -18 - 14 }, // * z^24
    +::boost::decimal::decimal64 { UINT64_C(122677862823826079), -18 - 14 }, // * z^25
    -::boost::decimal::decimal64 { UINT64_C(118125930169745877), -18 - 15 }, // * z^26
}};

} // namespace detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto tgamma(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    T result { };

    const auto nx = static_cast<int>(x);

    const auto is_pure_int = (nx == x);

    const auto fpc = fpclassify(x);

    if (fpc != FP_NORMAL)
    {
      if (fpc == FP_ZERO)
      {
          result = (signbit(x) ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity());
      }
      else if(fpc == FP_INFINITE)
      {
          result = (signbit(x) ? std::numeric_limits<T>::quiet_NaN() : std::numeric_limits<T>::infinity());
      }
      else
      {
          result = x;
      }
    }
    else if ((nx < 0) && is_pure_int && ((nx & 1) != 0))
    {
        // Pure negative integer argument.
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else
    {
        if (signbit(x))
        {
            // Reflection for negative argument.
            const auto ga = tgamma(-x);

            result = -numbers::pi_v<T> / ((x * ga) * sin(numbers::pi_v<T> * x));
        }
        else
        {
            constexpr T one { 1, 0 };

            if (is_pure_int)
            {
                result = one;

                for(auto index = 2; index < nx; ++index)
                {
                    result *= index;
                }
            }
            else
            {
                const auto x_is_gt_one = (x > one);

                auto r = one;

                auto z = x;

                if (x_is_gt_one)
                {
                    // Use a scaling for arguments greater than one.
                    for(auto k = 1; k <= nx; ++k)
                    {
                        r = r * (z - k);
                    }

                    z = z - nx;
                }

                // Perform the Taylor series expansion.
                auto rit = detail::coefs_array_table<T, sizeof(T) == static_cast<std::size_t>(UINT8_C(4)) ? static_cast<std::size_t>(UINT8_C(13)) : static_cast<std::size_t>(UINT8_C(25))>.crbegin();

                result = *rit;

                while(rit != detail::coefs_array_table<T, sizeof(T) == static_cast<std::size_t>(UINT8_C(4)) ? static_cast<std::size_t>(UINT8_C(13)) : static_cast<std::size_t>(UINT8_C(25))>.crend())
                {
                    result = fma(result, z, *rit++);
                }

                result = one / (z * fma(result, z, one));

                if (x_is_gt_one)
                {
                    // Use a scaling for arguments greater than one.
                    result *= r;
                }
            }
        }
    }

    return result;
}

/*

Coefficient list for 128-bit

C
C     N[Series[1/Gamma[z], {z, 0, 48}], 51]
C     Table[SeriesCoefficient[%, n], {n, 48}]
C     ... and the Series generation takes several minutes.
C     Don't concatenate these (potentially nested) operations,
C     as that did not end at all, at least not for me.
C
      DATA G/+1.0_QP,
     &  +5.77215664901532860606512090082402431042159335939924E-01_QP,
     &  -6.55878071520253881077019515145390481279766380478584E-01_QP,
     &  -4.20026350340952355290039348754298187113945004011061E-02_QP,
     &  +1.66538611382291489501700795102105235717781502247174E-01_QP,
     &  -4.21977345555443367482083012891873913016526841898225E-02_QP,
     &  -9.62197152787697356211492167234819897536294225211300E-03_QP,
     &  +7.21894324666309954239501034044657270990480088023832E-03_QP,
     &  -1.16516759185906511211397108401838866680933379538406E-03_QP,
     &  -2.15241674114950972815729963053647806478241923378339E-04_QP,
     &  +1.28050282388116186153198626328164323394892099693677E-04_QP,
     &  -2.01348547807882386556893914210218183822948332979791E-05_QP,
     &  -1.25049348214267065734535947383309224232265562115396E-06_QP,
     &  +1.13302723198169588237412962033074494332400483862108E-06_QP,
     &  -2.05633841697760710345015413002057283651257902629338E-07_QP,
     &  +6.11609510448141581786249868285534286727586571971232E-09_QP,
     &  +5.00200764446922293005566504805999130304461274249448E-09_QP,
     &  -1.18127457048702014458812656543650557773875950493259E-09_QP,
     &  +1.04342671169110051049154033231225019140070982312581E-10_QP,
     &  +7.78226343990507125404993731136077722606808618139294E-12_QP,
     &  -3.69680561864220570818781587808576623657096345136100E-12_QP,
     &  +5.10037028745447597901548132286323180272688606970763E-13_QP,
     &  -2.05832605356650678322242954485523741974609108081015E-14_QP,
     &  -5.34812253942301798237001731872793994898971547812068E-15_QP,
     &  +1.22677862823826079015889384662242242816545575045632E-15_QP,
     &  -1.18125930169745876951376458684229783121155729180485E-16_QP,
     &  +1.18669225475160033257977724292867407108849407966483E-18_QP,
     &  +1.41238065531803178155580394756670903708635075033453E-18_QP,
     &  -2.29874568443537020659247858063369926028450593141904E-19_QP,
     &  +1.71440632192733743338396337026725706681265606251743E-20_QP,
     &  +1.33735173049369311486478139512226802287505947176189E-22_QP,
     &  -2.05423355176667278932502535135573379668203793523874E-22_QP,
     &  +2.73603004860799984483150990433098201486531169583636E-23_QP,
     &  -1.73235644591051663905742845156477979906974910879500E-24_QP,
     &  -2.36061902449928728734345073542753100792641355214537E-26_QP,
     &  +1.86498294171729443071841316187866689894586842907367E-26_QP,
     &  -2.21809562420719720439971691362686037973177950067568E-27_QP,
     &  +1.29778197494799366882441448633059416561949986463913E-28_QP,
     &  +1.18069747496652840622274541550997151855968463784158E-30_QP,
     &  -1.12458434927708809029365467426143951211941179558301E-30_QP,
     &  +1.27708517514086620399020667775112464774877206560048E-31_QP,
     &  -7.39145116961514082346128933010855282371056899245153E-33_QP,
     &  +1.13475025755421576095416525946930639300861219592633E-35_QP,
     &  +4.63913464105872202994480490795222846305796867972715E-35_QP,
     &  -5.34733681843919887507741819670989332090488590577356E-36_QP,
     &  +3.20799592361335262286123727908279439109014635972616E-37_QP,
     &  -4.44582973655075688210159035212464363740143668574872E-39_QP,
     &  -1.31117451888198871290105849438992219023662544955743E-39 /

*/



} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP
