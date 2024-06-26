// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_LOCALE_CONVERSION_HPP
#define BOOST_DECIMAL_DETAIL_LOCALE_CONVERSION_HPP

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <locale>
#include <clocale>
#include <cstring>
#endif

namespace boost {
namespace decimal {
namespace detail {

inline void convert_string_to_c_locale(char* buffer) noexcept
{
    const auto locale_decimal_point = *std::localeconv()->decimal_point;
    if (locale_decimal_point != '.')
    {
        auto p = std::strchr(buffer, static_cast<int>(locale_decimal_point));
        if (p != nullptr)
        {
            *p = '.';
        }
    }
}

inline void convert_string_to_local_locale(char* buffer) noexcept
{
    const auto locale_decimal_point = *std::localeconv()->decimal_point;
    if (locale_decimal_point != '.')
    {
        auto p = std::strchr(buffer, static_cast<int>('.'));
        if (p != nullptr)
        {
            *p = locale_decimal_point;
        }
    }
}

inline void convert_pointer_pair_to_local_locale(char* first, const char* last) noexcept
{
    const auto locale_decimal_point = *std::localeconv()->decimal_point;
    if (locale_decimal_point != '.')
    {
        while (first != last)
        {
            if (*first == '.')
            {
                *first = locale_decimal_point;
            }

            ++first;
        }
    }
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_LOCALE_CONVERSION_HPP
