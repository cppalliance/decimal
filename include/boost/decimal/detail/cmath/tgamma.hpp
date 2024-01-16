// Copyright 2023 - 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP

#include <array>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/sin.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost {
namespace decimal {

namespace detail {
namespace tgamma {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto coefs_size = static_cast<std::size_t>(UINT8_C(0));

template <>
constexpr auto coefs_size<::boost::decimal::decimal32> = static_cast<std::size_t>(UINT8_C(13));

template <>
constexpr auto coefs_size<::boost::decimal::decimal64> = static_cast<std::size_t>(UINT8_C(25));

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T,
          const std::size_t CoefsSize = coefs_size<T>>
constexpr std::array<T, CoefsSize> coefs_array_table = {{ T { } }};

template <>
constexpr std::array<::boost::decimal::decimal32, coefs_size<::boost::decimal::decimal32>> coefs_array_table<::boost::decimal::decimal32, coefs_size<::boost::decimal::decimal32>> =
{{
    // N[Series[1/Gamma[z], {z, 0, 14}], 24]
    +::boost::decimal::decimal32 { UINT64_C(5'772'156'649'015'328'606), - 19 - 0 }, // * z^2
    -::boost::decimal::decimal32 { UINT64_C(6'558'780'715'202'538'811), - 19 - 0 }, // * z^3
    -::boost::decimal::decimal32 { UINT64_C(4'200'263'503'409'523'553), - 19 - 1 }, // * z^4
    +::boost::decimal::decimal32 { UINT64_C(1'665'386'113'822'914'895), - 19 - 0 }, // * z^5
    -::boost::decimal::decimal32 { UINT64_C(4'219'773'455'554'433'675), - 19 - 1 }, // * z^6
    -::boost::decimal::decimal32 { UINT64_C(9'621'971'527'876'973'562), - 19 - 2 }, // * z^7
    +::boost::decimal::decimal32 { UINT64_C(7'218'943'246'663'099'542), - 19 - 2 }, // * z^8
    -::boost::decimal::decimal32 { UINT64_C(1'165'167'591'859'065'112), - 19 - 2 }, // * z^9
    -::boost::decimal::decimal32 { UINT64_C(2'152'416'741'149'509'728), - 19 - 3 }, // * z^10
    +::boost::decimal::decimal32 { UINT64_C(1'280'502'823'881'161'862), - 19 - 3 }, // * z^11
    -::boost::decimal::decimal32 { UINT64_C(2'013'485'478'078'823'866), - 19 - 4 }, // * z^12
    -::boost::decimal::decimal32 { UINT64_C(1'250'493'482'142'670'657), - 19 - 5 }, // * z^13
    +::boost::decimal::decimal32 { UINT64_C(1'133'027'231'981'695'882), - 19 - 5 }, // * z^14
}};

template <>
constexpr std::array<::boost::decimal::decimal64, coefs_size<::boost::decimal::decimal64>> coefs_array_table<::boost::decimal::decimal64, coefs_size<::boost::decimal::decimal64>> =
{{
    // N[Series[1/Gamma[z], {z, 0, 26}], 24]
    +::boost::decimal::decimal64 { UINT64_C(5'772'156'649'015'328'606), -19 - 0  }, // * z^2
    -::boost::decimal::decimal64 { UINT64_C(6'558'780'715'202'538'811), -19 - 0  }, // * z^3
    -::boost::decimal::decimal64 { UINT64_C(4'200'263'503'409'523'553), -19 - 1  }, // * z^4
    +::boost::decimal::decimal64 { UINT64_C(1'665'386'113'822'914'895), -19 - 0  }, // * z^5
    -::boost::decimal::decimal64 { UINT64_C(4'219'773'455'554'433'675), -19 - 1  }, // * z^6
    -::boost::decimal::decimal64 { UINT64_C(9'621'971'527'876'973'562), -19 - 2  }, // * z^7
    +::boost::decimal::decimal64 { UINT64_C(7'218'943'246'663'099'542), -19 - 2  }, // * z^8
    -::boost::decimal::decimal64 { UINT64_C(1'165'167'591'859'065'112), -19 - 2  }, // * z^9
    -::boost::decimal::decimal64 { UINT64_C(2'152'416'741'149'509'728), -19 - 3  }, // * z^10
    +::boost::decimal::decimal64 { UINT64_C(1'280'502'823'881'161'862), -19 - 3  }, // * z^11
    -::boost::decimal::decimal64 { UINT64_C(2'013'485'478'078'823'866), -19 - 4  }, // * z^12
    -::boost::decimal::decimal64 { UINT64_C(1'250'493'482'142'670'657), -19 - 5  }, // * z^13
    +::boost::decimal::decimal64 { UINT64_C(1'133'027'231'981'695'882), -19 - 5  }, // * z^14
    -::boost::decimal::decimal64 { UINT64_C(2'056'338'416'977'607'103), -19 - 6  }, // * z^15
    +::boost::decimal::decimal64 { UINT64_C(6'116'095'104'481'415'818), -19 - 8  }, // * z^16
    +::boost::decimal::decimal64 { UINT64_C(5'002'007'644'469'222'930), -19 - 8  }, // * z^17
    -::boost::decimal::decimal64 { UINT64_C(1'181'274'570'487'020'145), -19 - 8  }, // * z^18
    +::boost::decimal::decimal64 { UINT64_C(1'043'426'711'691'100'510), -19 - 9  }, // * z^19
    +::boost::decimal::decimal64 { UINT64_C(7'782'263'439'905'071'254), -19 - 11 }, // * z^20
    -::boost::decimal::decimal64 { UINT64_C(3'696'805'618'642'205'708), -19 - 11 }, // * z^21
    +::boost::decimal::decimal64 { UINT64_C(5'100'370'287'454'475'979), -19 - 12 }, // * z^22
    -::boost::decimal::decimal64 { UINT64_C(2'058'326'053'566'506'783), -19 - 13 }, // * z^23
    -::boost::decimal::decimal64 { UINT64_C(5'348'122'539'423'017'982), -19 - 14 }, // * z^24
    +::boost::decimal::decimal64 { UINT64_C(1'226'778'628'238'260'790), -19 - 14 }, // * z^25
    -::boost::decimal::decimal64 { UINT64_C(1'181'259'301'697'458'770), -19 - 15 }, // * z^26
}};

} // namespace tgamma
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
                    // Use scaling for arguments greater than one.
                    // TODO(ckormanyos) This upscaling can get excessive for large argument.
                    // TODO(ckormanyos) For large argument (above a cutoff) use asymptotic expansion.

                    for(auto k = 1; k <= nx; ++k)
                    {
                        r = r * (z - k);
                    }

                    z = z - nx;
                }

                using detail::tgamma::coefs_array_table;

                // Perform the Taylor series expansion.
                auto rit = coefs_array_table<T>.crbegin();

                result = *rit;

                while(rit != coefs_array_table<T>.crend())
                {
                    result = fma(result, z, *rit++);
                }

                result = one / (z * fma(result, z, one));

                if (x_is_gt_one)
                {
                    // Downscale: From using scaling for arguments greater than one.
                    // TODO(ckormanyos) See related notes above for large argument.

                    result *= r;
                }
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP
