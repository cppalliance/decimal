// Copyright 2020-2023 Junekey Jeon
// Copyright 2022 Peter Dimov
// Copyright 2023-2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_TO_CHARS_INTEGER_IMPL_HPP
#define BOOST_TO_CHARS_INTEGER_IMPL_HPP

#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/to_chars_result.hpp>
#include <boost/decimal/detail/memcpy.hpp>
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

namespace boost {
namespace decimal {
namespace detail {

static constexpr char digit_table[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z'
};

// Use a simple lookup table to put together the Integer in character form
template <typename Integer, typename Unsigned_Integer>
BOOST_DECIMAL_CONSTEXPR auto to_chars_integer_impl(char* first, char* last, Integer value, int base) noexcept -> to_chars_result
{
    const std::ptrdiff_t output_length = last - first;

    if (!((first <= last) && (base >= 2 && base <= 36)))
    {
        return {last, std::errc::invalid_argument};
    }

    if (value == 0)
    {
        *first++ = '0';
        return {first, std::errc()};
    }

    Unsigned_Integer unsigned_value {};
    const auto unsigned_base = static_cast<Unsigned_Integer>(base);

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (value < 0)
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

    constexpr Unsigned_Integer zero = 48U; // Char for '0'
    constexpr auto buffer_size = sizeof(Unsigned_Integer) * CHAR_BIT;
    char buffer[buffer_size] {};
    const char* buffer_end = buffer + buffer_size;
    char* end = buffer + buffer_size - 1;

    // Work from LSB to MSB
    switch (base)
    {
        case 2:
            while (unsigned_value != 0)
            {
                *end-- = static_cast<char>(zero + (unsigned_value & 1U)); // 1<<1 - 1
                unsigned_value >>= 1U;
            }
            break;

        case 4:
            while (unsigned_value != 0)
            {
                *end-- = static_cast<char>(zero + (unsigned_value & 3U)); // 1<<2 - 1
                unsigned_value >>= 2U;
            }
            break;

        case 8:
            while (unsigned_value != 0)
            {
                *end-- = static_cast<char>(zero + (unsigned_value & 7U)); // 1<<3 - 1
                unsigned_value >>= 3U;
            }
            break;

        case 16:
            while (unsigned_value != 0)
            {
                *end-- = digit_table[static_cast<std::size_t>(unsigned_value & 15U)]; // 1<<4 - 1
                unsigned_value >>= 4U;
            }
            break;

        case 32:
            while (unsigned_value != 0)
            {
                *end-- = digit_table[static_cast<std::size_t>(unsigned_value & 31U)]; // 1<<5 - 1
                unsigned_value >>= 5U;
            }
            break;

        default:
            while (unsigned_value != 0)
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
