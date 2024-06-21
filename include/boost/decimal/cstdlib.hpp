// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDLIB_HPP
#define BOOST_DECIMAL_CSTDLIB_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/locale_conversion.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <memory>
#include <new>
#include <limits>
#include <locale>
#include <cstdint>
#include <clocale>
#endif

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

namespace boost {
namespace decimal {

namespace detail {

// 3.8.2
template <typename TargetDecimalType>
inline auto strtod_calculation(const char* str, char** endptr, char* buffer, std::size_t str_length) noexcept -> TargetDecimalType
{
    using significand_type = std::conditional_t<std::is_same<TargetDecimalType, decimal128>::value ||
                                                std::is_same<TargetDecimalType, decimal128_fast>::value, detail::uint128, std::uint64_t>;

    std::memcpy(buffer, str, str_length);
    convert_string_to_c_locale(buffer);

    bool sign {};
    significand_type significand {};
    std::int32_t expval {};

    const auto r {detail::parser(buffer, buffer + str_length, sign, significand, expval)};
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
        *endptr = const_cast<char*>(str + (r.ptr - buffer));
    }

    return d;
}

template <typename TargetDecimalType>
inline auto strtod_impl(const char* str, char** endptr) noexcept -> TargetDecimalType
{
    if (str == nullptr)
    {
        errno = EINVAL;
        return std::numeric_limits<TargetDecimalType>::quiet_NaN();
    }

    const auto str_length {std::strlen(str)};

    if (str_length < 1024U)
    {
        char buffer[1024U];
        return strtod_calculation<TargetDecimalType>(str, endptr, buffer, str_length);
    }

    // If the string to be parsed does not fit into the 1024 byte static buffer than we have to allocate a buffer.
    // malloc is used here because it does not throw on allocation failure.
    std::unique_ptr<char[]> buffer(new(std::nothrow) char[str_length + 1]);
    if (buffer == nullptr)
    {
        // Hard to get coverage on memory exhaustion
        // LCOV_EXCL_START
        errno = ENOMEM;
        return std::numeric_limits<TargetDecimalType>::quiet_NaN();
        // LCOV_EXCL_STOP
    }

    auto d = strtod_calculation<TargetDecimalType>(str, endptr, buffer.get(), str_length);

    return d;
}

// 3.9.2
template <typename TargetDecimalType>
inline auto wcstod_calculation(const wchar_t* str, wchar_t** endptr, char* buffer, std::size_t str_length) noexcept -> TargetDecimalType
{
    // Convert all the characters from wchar_t to char and use regular strtod32
    for (std::size_t i {}; i < str_length; ++i)
    {
        auto val {*(str + i)};
        if (BOOST_DECIMAL_UNLIKELY(val > 255))
        {
            // Character can not be converted
            return std::numeric_limits<TargetDecimalType>::quiet_NaN();
        }

        buffer[i] = static_cast<char>(val);
    }

    *(buffer + str_length) = '\0';
    char* short_endptr {};
    const auto return_val {strtod_impl<TargetDecimalType>(buffer, &short_endptr)};

    if (endptr != nullptr)
    {
        *endptr = const_cast<wchar_t*>(str + (short_endptr - buffer));
    }

    return return_val;
}

template <typename TargetDecimalType>
inline auto wcstod_impl(const wchar_t* str, wchar_t** endptr) noexcept -> TargetDecimalType
{
    if (str == nullptr)
    {
        errno = EINVAL;
        return std::numeric_limits<TargetDecimalType>::quiet_NaN();
    }

    const auto str_length {detail::strlen(str)};

    if (str_length < 1024U)
    {
        char buffer[1024U];
        return wcstod_calculation<TargetDecimalType>(str, endptr, buffer, str_length);
    }

    // If the string to be parsed does not fit into the 1024 byte static buffer than we have to allocate a buffer.
    // malloc is used here because it does not throw on allocation failure.
    std::unique_ptr<char[]> buffer(new(std::nothrow) char[str_length + 1]);
    if (buffer == nullptr)
    {
        // Hard to get coverage on memory exhaustion
        // LCOV_EXCL_START
        errno = ENOMEM;
        return std::numeric_limits<TargetDecimalType>::quiet_NaN();
        // LCOV_EXCL_STOP
    }

    return wcstod_calculation<TargetDecimalType>(str, endptr, buffer.get(), str_length);
}

} //namespace detail

BOOST_DECIMAL_EXPORT template <typename TargetDecimalType = decimal64>
inline auto strtod(const char* str, char** endptr) noexcept -> TargetDecimalType
{
    return detail::strtod_impl<TargetDecimalType>(str, endptr);
}

BOOST_DECIMAL_EXPORT template <typename TargetDecimalType = decimal64>
inline auto wcstod(const wchar_t* str, wchar_t** endptr) noexcept -> TargetDecimalType
{
    return detail::wcstod_impl<TargetDecimalType>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto strtod32(const char* str, char** endptr) noexcept -> decimal32
{
    return detail::strtod_impl<decimal32>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto wcstod32(const wchar_t* str, wchar_t** endptr) noexcept -> decimal32
{
    return detail::wcstod_impl<decimal32>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto strtod32f(const char* str, char** endptr) noexcept -> decimal32_fast
{
    return detail::strtod_impl<decimal32_fast>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto wcstod32f(const wchar_t* str, wchar_t** endptr) noexcept -> decimal32_fast
{
    return detail::wcstod_impl<decimal32_fast>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto strtod64(const char* str, char** endptr) noexcept -> decimal64
{
    return detail::strtod_impl<decimal64>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto wcstod64(const wchar_t* str, wchar_t** endptr) noexcept -> decimal64
{
    return detail::wcstod_impl<decimal64>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto strtod64f(const char* str, char** endptr) noexcept -> decimal64_fast
{
    return detail::strtod_impl<decimal64_fast>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto wcstod64f(const wchar_t* str, wchar_t** endptr) noexcept -> decimal64_fast
{
    return detail::wcstod_impl<decimal64_fast>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto strtod128(const char* str, char** endptr) noexcept -> decimal128
{
    return detail::strtod_impl<decimal128>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto wcstod128(const wchar_t* str, wchar_t** endptr) noexcept -> decimal128
{
    return detail::wcstod_impl<decimal128>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto strtod128f(const char* str, char** endptr) noexcept -> decimal128_fast
{
    return detail::strtod_impl<decimal128_fast>(str, endptr);
}

BOOST_DECIMAL_EXPORT inline auto wcstod128f(const wchar_t* str, wchar_t** endptr) noexcept -> decimal128_fast
{
    return detail::wcstod_impl<decimal128_fast>(str, endptr);
}

} // namespace decimal
} // namespace boost

#endif

#endif // BOOST_DECIMAL_CSTDLIB_HPP
