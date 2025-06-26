// Copyright 2020-2023 Junekey Jeon
// Copyright 2022 Peter Dimov
// Copyright 2023-2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_TO_CHARS_INTEGER_IMPL_HPP
#define BOOST_TO_CHARS_INTEGER_IMPL_HPP

#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/int128.hpp>
#include <boost/decimal/detail/to_chars_result.hpp>
#include <boost/decimal/detail/memcpy.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <system_error>
#include <type_traits>
#include <array>
#include <limits>
#include <utility>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdint>
#include <climits>
#include <cmath>
#endif

namespace boost {
namespace decimal {
namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE char digit_table[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z'
};

// Use a simple lookup table to put together the Integer in character form
template <typename Integer, typename Unsigned_Integer>
BOOST_DECIMAL_CONSTEXPR auto to_chars_integer_impl(char* first, char* last, Integer value, int base) noexcept
    BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_integral_v, Integer, detail::is_integral_v, Unsigned_Integer, to_chars_result)
{
    const std::ptrdiff_t output_length = last - first;

    if (!((first <= last) && (base >= 2 && base <= 36)))
    {
        return {last, std::errc::invalid_argument};
    }

    if (value == 0U)
    {
        *first++ = '0';
        return {first, std::errc()};
    }

    Unsigned_Integer unsigned_value {};
    const auto unsigned_base = static_cast<Unsigned_Integer>(base);

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (value < static_cast<Integer>(0))
        {
            *first++ = '-';
            unsigned_value = static_cast<Unsigned_Integer>(detail::apply_sign(value));
        }
        else
        {
            unsigned_value = static_cast<Unsigned_Integer>(value);
        }
    }
    else
    {
        unsigned_value = static_cast<Unsigned_Integer>(value);
    }

    constexpr auto buffer_size = sizeof(Unsigned_Integer) * CHAR_BIT;
    char buffer[buffer_size] {};
    const char* buffer_end = buffer + buffer_size;
    char* end = buffer + buffer_size - 1;

    // Work from LSB to MSB
    switch (base)
    {
        case 16:
            while (unsigned_value != 0U)
            {
                *end-- = digit_table[static_cast<std::size_t>(unsigned_value & 15U)]; // 1<<4 - 1
                unsigned_value >>= 4U;
            }
            break;

        default:
            while (unsigned_value != 0U)
            {
                *end-- = digit_table[static_cast<std::size_t>(unsigned_value % unsigned_base)];
                unsigned_value /= unsigned_base;
            }
            break;
    }

    const std::ptrdiff_t num_chars = buffer_end - end - 1;

    if (num_chars > output_length)
    {
        return {last, std::errc::value_too_large};
    }

    boost::decimal::detail::memcpy(first, buffer + (buffer_size - static_cast<unsigned long>(num_chars)), static_cast<std::size_t>(num_chars));

    return {first + num_chars, std::errc()};
}

}
}
}

#endif //BOOST_TO_CHARS_INTEGER_IMPL_HPP
