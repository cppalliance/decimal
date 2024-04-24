// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDIO_HPP
#define BOOST_DECIMAL_CSTDIO_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/locale_conversion.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/charconv.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <memory>
#include <new>
#include <cctype>
#include <cstdio>
#endif

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

// H is the type modifier used for decimal32
// D is the type modifier used for deciaml64
// DD is the type modifier used for decimal128

namespace boost {
namespace decimal {

namespace detail {

enum class decimal_type : unsigned
{
    decimal32 = 1 << 0,
    decimal64 = 1 << 1,
    decimal128 = 1 << 2
};

struct parameters
{
    int precision;
    chars_format fmt;
    decimal_type return_type;
    bool upper_case;
};

inline auto parse_format(const char* format) -> parameters
{
    // If the format is unspecified or incorrect we will use this as the default values
    parameters params {6, chars_format::general, decimal_type::decimal64, false};

    auto iter = format;
    const auto last = format + std::strlen(format);

    if (iter == last || *iter != '%')
    {
        return params;
    }

    ++iter;
    if (iter == last)
    {
        return params;
    }

    // Case where we have a precision argumet
    if (*iter == '.')
    {
        ++iter;
        if (iter == last)
        {
            return params;
        }

        params.precision = 0;
        while (iter != last && is_integer_char(*iter))
        {
            params.precision = params.precision * 10 + digit_from_char(*iter);
            ++iter;
        }

        if (iter == last)
        {
            return params;
        }
    }

    // Now get the type specifier
    if (*iter == 'H')
    {
        params.return_type = decimal_type::decimal32;
        ++iter;
    }
    else if (*iter == 'D')
    {
        ++iter;
        if (iter == last)
        {
            return params;
        }
        else if (*iter == 'D')
        {
            params.return_type = decimal_type::decimal128;
            ++iter;
        }
    }

    // And lastly the format
    if (iter == last)
    {
        return params;
    }

    switch (*iter)
    {
        case 'G':
            params.upper_case = true;
            break;
        case 'E':
            params.upper_case = true;
            params.fmt = chars_format::scientific;
            break;
        case 'e':
            params.fmt = chars_format::scientific;
            break;
        case 'f':
            params.fmt = chars_format::fixed;
            break;
        case 'A':
            params.upper_case = true;
            params.fmt = chars_format::hex;
            break;
        case 'a':
            params.fmt = chars_format::hex;
            break;
        default:
            // Invalid specifier
            return params;
    }

    return params;
}

inline void make_uppercase(char* first, const char* last) noexcept
{
    while (first != last)
    {
        if ((*first >= 'a' && *first <= 'f') || *first == 'p')
        {
            *first = static_cast<char>(std::toupper(static_cast<int>(*first)));
        }
        ++first;
    }
}

template <typename... T>
inline auto snprintf_impl(char* buffer, std::size_t buf_size, const char* format, T... values) noexcept
    #ifndef BOOST_DECIMAL_HAS_CONCEPTS
    -> std::enable_if_t<detail::is_decimal_floating_point_v<std::common_type_t<T...>>, int>
    #else
    -> int requires detail::is_decimal_floating_point_v<std::common_type_t<T...>>
    #endif
{
    if (buffer == nullptr || format == nullptr)
    {
        return -1;
    }

    std::size_t byte_count {};
    const std::initializer_list<std::common_type_t<T...>> values_list {values...};
    auto value_iter = values_list.begin();
    const char* iter {format};
    const char* buffer_begin {buffer};

    const auto format_size = std::strlen(format);

    if (*iter == '"')
    {
        ++iter;
    }

    while (byte_count < format_size)
    {
        while (byte_count < format_size && *iter != '%')
        {
            *buffer++ = *iter++;
            ++byte_count;
        }

        if (byte_count == format_size)
        {
            break;
        }

        char params_buffer[10] {};
        std::size_t param_iter {};
        while (byte_count < format_size && *iter != ' ' && *iter != '"')
        {
            params_buffer[param_iter] = *iter++;
            ++byte_count;
            ++param_iter;
        }

        const auto params = parse_format(params_buffer);
        to_chars_result r;
        switch (params.return_type)
        {
            // Subtract 1 from all cases to ensure there is room to insert the null terminator
            case detail::decimal_type::decimal32:
                r = to_chars(buffer, buffer + buf_size - byte_count, static_cast<decimal32>(*value_iter), params.fmt, params.precision);
                break;
            case detail::decimal_type::decimal64:
                r = to_chars(buffer, buffer + buf_size - byte_count, static_cast<decimal64>(*value_iter), params.fmt, params.precision);
                break;
            default:
                r = to_chars(buffer, buffer + buf_size - byte_count, static_cast<decimal128>(*value_iter), params.fmt, params.precision);
                break;
        }

        if (!r)
        {
            errno = static_cast<int>(r.ec);
            return -1;
        }

        buffer = r.ptr;

        if (value_iter != values_list.end())
        {
            ++value_iter;
        }
    }

    *buffer = '\0';
    return static_cast<int>(buffer - buffer_begin);
}

} // namespace detail

template <typename... T>
inline auto snprintf(char* buffer, std::size_t buf_size, const char* format, T... values) noexcept
    #ifndef BOOST_DECIMAL_HAS_CONCEPTS
    -> std::enable_if_t<detail::is_decimal_floating_point_v<std::common_type_t<T...>>, int>
    #else
    -> int requires detail::is_decimal_floating_point_v<std::common_type_t<T...>>
    #endif
{
    return detail::snprintf_impl(buffer, buf_size, format, values...);
}

template <typename... T>
inline auto sprintf(char* buffer, const char* format, T... values) noexcept
    #ifndef BOOST_DECIMAL_HAS_CONCEPTS
    -> std::enable_if_t<detail::is_decimal_floating_point_v<std::common_type_t<T...>>, int>
    #else
    -> int requires detail::is_decimal_floating_point_v<std::common_type_t<T...>>
    #endif
{
    return detail::snprintf_impl(buffer, sizeof(buffer), format, values...);
}

template <typename... T>
inline auto fprintf(std::FILE* buffer, const char* format, T... values) noexcept
#ifndef BOOST_DECIMAL_HAS_CONCEPTS
    -> std::enable_if_t<detail::is_decimal_floating_point_v<std::common_type_t<T...>>, int>
    #else
    -> int requires detail::is_decimal_floating_point_v<std::common_type_t<T...>>
    #endif
{
    if (format == nullptr)
    {
        return -1;
    }

    const auto format_len{std::strlen(format)};
    int bytes {};
    char char_buffer[1024];

    if (format_len <= 1024U)
    {
        bytes = detail::snprintf_impl(char_buffer, sizeof(char_buffer), format, values...);
        if (bytes)
        {
            bytes += std::fwrite(char_buffer, sizeof(char), static_cast<std::size_t>(bytes), buffer);
        }
    }
    else
    {
        std::unique_ptr<char[]> longer_char_buffer(new(std::nothrow) char[format_len + 1]);
        if (buffer == nullptr)
        {
            // Hard to get coverage on memory exhaustion
            // LCOV_EXCL_START
            errno = ENOMEM;
            return -1;
            // LCOV_EXCL_STOP
        }

        bytes = detail::snprintf_impl(longer_char_buffer.get(), format_len, format, values...);
        if (bytes)
        {
            bytes += std::fwrite(longer_char_buffer.get(), sizeof(char), static_cast<std::size_t>(bytes), buffer);
        }
    }

    return bytes;
}

template <typename... T>
inline auto printf(const char* format, T... values) noexcept
    #ifndef BOOST_DECIMAL_HAS_CONCEPTS
    -> std::enable_if_t<detail::is_decimal_floating_point_v<std::common_type_t<T...>>, int>
    #else
    -> int requires detail::is_decimal_floating_point_v<std::common_type_t<T...>>
    #endif
{
    return fprintf(stdout, format, values...);
}

} // namespace decimal
} // namespace boost

#endif // #if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#endif //BOOST_DECIMAL_CSTDIO_HPP
