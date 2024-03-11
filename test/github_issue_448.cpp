// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/cppalliance/decimal/issues/448

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>
#include <cstring>

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif


template <typename T>
void test_type_traits()
{
    static_assert(std::is_trivially_copyable<T>::value, "Not trivially copyable");
    static_assert(std::is_trivially_copy_constructible<T>::value, "Not trivially copy constructible");
    static_assert(std::is_trivially_move_constructible<T>::value, "Not trivially move constructible");
}

template <typename T>
void test_memset()
{
    struct test_struct
    {
        T quantity;
    };

    test_struct test {};
    std::memset(&test, 0, sizeof(test_struct));

    BOOST_TEST_EQ(test.quantity, T{});
}

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

int main()
{
    test_type_traits<boost::decimal::decimal32>();
    test_type_traits<boost::decimal::decimal64>();
    test_type_traits<boost::decimal::decimal128>();

    test_memset<boost::decimal::decimal32>();
    test_memset<boost::decimal::decimal64>();
    test_memset<boost::decimal::decimal128>();

    return boost::report_errors();
}
