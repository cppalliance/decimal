// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_IO_HPP
#define BOOST_DECIMAL_DETAIL_IO_HPP

#include <cerrno>
#include <cstring>
#include <limits>
#include <iostream>
#include <type_traits>
#include <system_error>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/parser.hpp>

namespace boost {
namespace decimal {

template <typename charT, typename traits, typename DecimalType>
auto operator>>(std::basic_istream<charT, traits>& is, DecimalType& d)
    -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_istream<charT, traits>&>
{
    char buffer[1024] {}; // What should be an unreasonably high maximum
    is >> buffer;

    bool sign {};
    std::uint64_t significand {};
    std::int32_t expval {};
    const auto buffer_len {std::strlen(buffer)};

    if (buffer_len == 0)
    {
        errno = EINVAL;
        return is;
    }

    const auto r {detail::parser(buffer, buffer + buffer_len, sign, significand, expval)};

    if (r.ec != std::errc{})
    {
        if (r.ec == std::errc::not_supported)
        {
            if (significand)
            {
                d = std::numeric_limits<DecimalType>::signaling_NaN();
            }
            else
            {
                d = std::numeric_limits<DecimalType>::quiet_NaN();
            }
        }
        else if (r.ec == std::errc::value_too_large)
        {
            d = std::numeric_limits<DecimalType>::infinity();
        }
        else
        {
            d = std::numeric_limits<DecimalType>::signaling_NaN();
            errno = static_cast<int>(r.ec);
        }
    }
    else
    {
        d = DecimalType{significand, expval, sign};
    }

    return is;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_IO_HPP
