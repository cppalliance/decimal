// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDLIB_HPP
#define BOOST_DECIMAL_CSTDLIB_HPP

#include <cstdint>
#include <limits>
#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/utilities.hpp>

namespace boost {
namespace decimal {

namespace detail {

// 3.8.2
template <typename TargetDecimalType>
constexpr auto strtod_impl(const char* str, char** endptr) noexcept -> TargetDecimalType
{
    if (str == nullptr)
    {
        errno = EINVAL;
        return std::numeric_limits<TargetDecimalType>::signaling_NaN();
    }

    auto sign        = bool {};
    auto significand = std::uint64_t {};
    auto expval      = std::int32_t {};

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

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_CSTDLIB_HPP
