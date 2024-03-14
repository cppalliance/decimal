// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDLIB_HPP
#define BOOST_DECIMAL_CSTDLIB_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <memory>
#include <new>
#include <limits>
#include <locale>
#include <cstdint>
#include <clocale>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

namespace boost {
namespace decimal {

namespace detail {

// We know that the string is in the "C" locale because it would have previously passed through our parser.
// Convert the string into the current locale so that the strto* family of functions
// works correctly for the given locale.
//
// We are operating on our own copy of the buffer, so we are free to modify it.
inline void convert_string_locale(char* buffer) noexcept
{
    const auto locale_decimal_point = *std::localeconv()->decimal_point;
    if (locale_decimal_point != '.')
    {
        auto p = std::strchr(buffer, '.');
        if (p != nullptr)
        {
            *p = locale_decimal_point;
        }
    }
}

// 3.8.2
template <typename TargetDecimalType>
inline auto strtod_calculation(const char* str, char** endptr, char* buffer, std::size_t str_length) noexcept -> TargetDecimalType
{
    using significand_type = std::conditional_t<std::is_same<TargetDecimalType, decimal128>::value, detail::uint128, std::uint64_t>;

    if (str == nullptr)
    {
        errno = EINVAL;
        return std::numeric_limits<TargetDecimalType>::signaling_NaN();
    }

    bool sign {};
    significand_type significand {};
    std::int32_t expval {};

    const auto buffer_len {detail::strlen(str)};

    const auto r {detail::parser(str, str + buffer_len, sign, significand, expval)};
    TargetDecimalType d {};

    if (r.ec != std::errc{})
    {
        if (r.ec == std::errc::not_supported)
        {
            if (significand)
            {
                d = std::numeric_limits<TargetDecimalType>::signaling_NaN();
            }
            else
            {
                d = std::numeric_limits<TargetDecimalType>::quiet_NaN();
            }
        }
        else if (r.ec == std::errc::value_too_large)
        {
            d = std::numeric_limits<TargetDecimalType>::infinity();
        }
        else
        {
            d = std::numeric_limits<TargetDecimalType>::signaling_NaN();
            errno = static_cast<int>(r.ec);
        }
    }
    else
    {
        d = TargetDecimalType(significand, expval, sign);
    }

    if (endptr != nullptr)
    {
        *endptr = const_cast<char*>(str + (r.ptr - str));
    }

    return d;
}

// 3.9.2
template <typename TargetDecimalType>
constexpr auto wcstod_impl(const wchar_t* str, wchar_t** endptr) noexcept -> TargetDecimalType
{
    char buffer[1024] {};
    if (str == nullptr || detail::strlen(str) > sizeof(buffer))
    {
        errno = EINVAL;
        return std::numeric_limits<TargetDecimalType>::signaling_NaN();
    }

    // Convert all the characters from wchar_t to char and use regular strtod32
    for (std::size_t i {}; i < detail::strlen(str); ++i)
    {
        auto val {*(str + i)};
        if (BOOST_DECIMAL_UNLIKELY(val > 255))
        {
            // Character can not be converted
            break;
        }

        buffer[i] = static_cast<char>(val);
    }

    char* short_endptr {};
    const auto return_val {strtod_impl<TargetDecimalType>(buffer, &short_endptr)};

    if (endptr != nullptr)
    {
        *endptr = const_cast<wchar_t*>(str + (short_endptr - buffer));
    }

    return return_val;
}

} //namespace detail

template <typename TargetDecimalType = decimal64>
constexpr auto strtod(const char* str, char** endptr) noexcept -> TargetDecimalType
{
    return detail::strtod_impl<TargetDecimalType>(str, endptr);
}

template <typename TargetDecimalType = decimal64>
constexpr auto wcstod(const wchar_t* str, wchar_t** endptr) noexcept -> TargetDecimalType
{
    return detail::wcstod_impl<TargetDecimalType>(str, endptr);
}

constexpr auto strtod32(const char* str, char** endptr) noexcept -> decimal32
{
    return detail::strtod_impl<decimal32>(str, endptr);
}

constexpr auto wcstod32(const wchar_t* str, wchar_t** endptr) noexcept -> decimal32
{
    return detail::wcstod_impl<decimal32>(str, endptr);
}

constexpr auto strtod64(const char* str, char** endptr) noexcept -> decimal64
{
    return detail::strtod_impl<decimal64>(str, endptr);
}

constexpr auto wcstod64(const wchar_t* str, wchar_t** endptr) noexcept -> decimal64
{
    return detail::wcstod_impl<decimal64>(str, endptr);
}

constexpr auto strtod128(const char* str, char** endptr) noexcept -> decimal128
{
    return detail::strtod_impl<decimal128>(str, endptr);
}

constexpr auto wcstod128(const wchar_t* str, wchar_t** endptr) noexcept -> decimal128
{
    return detail::wcstod_impl<decimal128>(str, endptr);
}

} // namespace decimal
} // namespace boost

#endif

#endif // BOOST_DECIMAL_CSTDLIB_HPP
