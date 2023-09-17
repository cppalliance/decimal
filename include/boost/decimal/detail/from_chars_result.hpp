// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FROM_CHARS_RESULT_HPP
#define BOOST_DECIMAL_DETAIL_FROM_CHARS_RESULT_HPP

#include <system_error>

namespace boost { namespace decimal {

// 22.13.3, Primitive numerical input conversion

template <typename UC>
struct from_chars_result_t
{
    const UC* ptr;

    // Values:
    // 0 = no error
    // EINVAL = invalid_argument
    // ERANGE = result_out_of_range
    std::errc ec;

    friend constexpr auto operator==(const from_chars_result_t<UC>& lhs, const from_chars_result_t<UC>& rhs) noexcept -> bool
    {
        return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
    }

    friend constexpr auto operator!=(const from_chars_result_t<UC>& lhs, const from_chars_result_t<UC>& rhs) noexcept -> bool
    {
        return !(lhs == rhs); // NOLINT : Expression can not be simplified since this is the definition
    }
};
using from_chars_result = from_chars_result_t<char>;

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_FROM_CHARS_RESULT_HPP
