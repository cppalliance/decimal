// Copyright 2023 - 2024 Matt Borland
// Copyright 1999 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

// This header file has been taken from (and significantly reduced/simplified from):
//
///////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 1999 - 2024.                 //
//  Distributed under the Boost Software License,                //
//  Version 1.0. (See accompanying file LICENSE_1_0.txt          //
//  or copy at http://www.boost.org/LICENSE_1_0.txt)             //
///////////////////////////////////////////////////////////////////

#ifndef UINTWIDE_T_2018_10_02_H // NOLINT(llvm-header-guard)
#define UINTWIDE_T_2018_10_02_H

#define BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE boost::decimal

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/numeric_limits.hpp>
#include <boost/decimal/detail/type_traits.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <type_traits>
#endif

#if (defined(_MSC_VER) && (!defined(__GNUC__) && !defined(__clang__)))
  #if (_MSC_VER >= 1900) && defined(_HAS_CXX20) && (_HAS_CXX20 != 0)
    #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD [[nodiscard]]           // NOLINT(cppcoreguidelines-macro-usage)
  #else
    #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD
  #endif
#else
  #if (defined(__cplusplus) && (__cplusplus >= 201402L))
    #if defined(__AVR__) && (!defined(__GNUC__) || (defined(__GNUC__) && (__cplusplus >= 202002L)))
    #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD [[nodiscard]]           // NOLINT(cppcoreguidelines-macro-usage)
    #elif (defined(__cpp_lib_constexpr_algorithms) && (__cpp_lib_constexpr_algorithms >= 201806))
      #if defined(__clang__)
        #if (__clang_major__ > 9)
        #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD [[nodiscard]]           // NOLINT(cppcoreguidelines-macro-usage)
        #else
        #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD
        #endif
      #else
      #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD [[nodiscard]]           // NOLINT(cppcoreguidelines-macro-usage)
      #endif
    #elif (defined(__clang__) && (__clang_major__ >= 10)) && (defined(__cplusplus) && (__cplusplus > 201703L))
      #if defined(__x86_64__)
      #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD [[nodiscard]]           // NOLINT(cppcoreguidelines-macro-usage)
      #else
      #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD
      #endif
    #else
    #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD
    #endif
  #else
    #define BOOST_DECIMAL_WIDE_INTEGER_NODISCARD
  #endif
#endif

#define BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR constexpr                // NOLINT(cppcoreguidelines-macro-usage)
#define BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR_IS_COMPILE_TIME_CONST 1  // NOLINT(cppcoreguidelines-macro-usage)

#if defined(BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_BEGIN) || defined(BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_END)
  #error internal pre-processor macro already defined
#endif

#if defined(BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE)
  #define BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_BEGIN namespace boost { namespace decimal {     // NOLINT(cppcoreguidelines-macro-usage)
  #define BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_END } } // namespace BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE  // NOLINT(cppcoreguidelines-macro-usage)
#else
  #define BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_BEGIN
  #define BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_END
#endif

BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_BEGIN

// Clang-Cl does not like the use of flat arrays
#if defined(__clang__) && !defined(__GNUC__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

namespace math { namespace wide_integer {

namespace detail {

namespace iterator_detail {

class input_iterator_tag {};
class output_iterator_tag {};
class forward_iterator_tag : public input_iterator_tag {};
class bidirectional_iterator_tag : public forward_iterator_tag {};
class random_access_iterator_tag : public bidirectional_iterator_tag {};

template<typename iterator_type>
class iterator_traits
{
public:
  using difference_type   = typename iterator_type::difference_type;
  using value_type        = typename iterator_type::value_type;
  using pointer           = typename iterator_type::pointer;
  using reference         = typename iterator_type::reference;
  using iterator_category = typename iterator_type::iterator_category;
};

template<typename T>
class iterator_traits<T*>
{
public:
  using difference_type   = std::ptrdiff_t;
  using value_type        = T;
  using pointer           = value_type*;
  using reference         = value_type&;
  using iterator_category = random_access_iterator_tag;
};

template<typename T>
class iterator_traits<const T*>
{
public:
  using difference_type   = std::ptrdiff_t;
  using value_type        = T;
  using pointer           = const value_type*;
  using reference         = const value_type&;
  using iterator_category = random_access_iterator_tag;
};

template<typename my_category,
         typename my_value_type,
         typename my_difference_type = std::ptrdiff_t,
         typename my_pointer_type    = my_value_type*,
         typename my_reference_type  = my_value_type&>
struct my_iterator
{
  using iterator_category = my_category;
  using value_type        = my_value_type;
  using difference_type   = my_difference_type;
  using pointer           = my_pointer_type;
  using reference         = my_reference_type;

  constexpr my_iterator() = default;
};

template <typename iterator_type>
class reverse_iterator : public my_iterator<typename iterator_traits<iterator_type>::iterator_category,
                                            typename iterator_traits<iterator_type>::value_type,
                                            typename iterator_traits<iterator_type>::difference_type,
                                            typename iterator_traits<iterator_type>::pointer,
                                            typename iterator_traits<iterator_type>::reference>
{
public:
  using value_type        = typename iterator_traits<iterator_type>::value_type;
  using difference_type   = typename iterator_traits<iterator_type>::difference_type;
  using pointer           = typename iterator_traits<iterator_type>::pointer;
  using reference         = typename iterator_traits<iterator_type>::reference;
  using iterator_category = typename iterator_traits<iterator_type>::iterator_category;

  #ifdef BOOST_DECIMAL_ENABLE_CUDA
  #  pragma nv_diag_suppress 20012
  #endif

  BOOST_DECIMAL_GPU_ENABLED constexpr reverse_iterator() = default;

  #ifdef BOOST_DECIMAL_ENABLE_CUDA
  #  pragma nv_diag_default 20012
  #endif

  BOOST_DECIMAL_GPU_ENABLED explicit constexpr reverse_iterator(iterator_type x) : current(x) { }

  template<typename other>
  BOOST_DECIMAL_GPU_ENABLED constexpr reverse_iterator(const reverse_iterator<other>& u) : current(u.current) { } // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)

  BOOST_DECIMAL_GPU_ENABLED BOOST_DECIMAL_WIDE_INTEGER_NODISCARD constexpr auto base() const -> iterator_type { return current; }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator* () const -> reference { iterator_type tmp = current; return *--tmp; }
  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator->() const -> pointer   { return &(operator*()); }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator++() -> reverse_iterator& { --current; return *this; }
  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator--() -> reverse_iterator& { ++current; return *this; }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator++(int) -> reverse_iterator { reverse_iterator tmp = *this; --current; return tmp; }
  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator--(int) -> reverse_iterator { reverse_iterator tmp = *this; ++current; return tmp; }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(typename reverse_iterator<iterator_type>::difference_type n) const -> reverse_iterator { return reverse_iterator(current - n); }
  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(typename reverse_iterator<iterator_type>::difference_type n) const -> reverse_iterator { return reverse_iterator(current + n); }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+=(typename reverse_iterator<iterator_type>::difference_type n) -> reverse_iterator& { current -= n; return *this; }
  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-=(typename reverse_iterator<iterator_type>::difference_type n) -> reverse_iterator& { current += n; return *this; }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator[](typename reverse_iterator<iterator_type>::difference_type n) const -> reference { return current[-n - 1]; }

private:
  iterator_type current; // NOLINT(readability-identifier-naming)

  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator< (const reverse_iterator& x, const reverse_iterator& y) -> bool { return (x.current  > y.current); }
  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=(const reverse_iterator& x, const reverse_iterator& y) -> bool { return (x.current >= y.current); }
  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator==(const reverse_iterator& x, const reverse_iterator& y) -> bool { return (x.current == y.current); }
  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator!=(const reverse_iterator& x, const reverse_iterator& y) -> bool { return (x.current != y.current); }
  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>=(const reverse_iterator& x, const reverse_iterator& y) -> bool { return (x.current <= y.current); }
  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator> (const reverse_iterator& x, const reverse_iterator& y) -> bool { return (x.current <  y.current); }

  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(const reverse_iterator& x, const reverse_iterator& y) -> typename reverse_iterator::difference_type
  {
    return (y.current - x.current);
  }

  friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(typename reverse_iterator::difference_type n, const reverse_iterator& x) -> reverse_iterator
  {
    return reverse_iterator(x.current - n);
  }
};

} // namespace iterator_detail

// Use a local, constexpr, unsafe implementation of the fill-function.
template<typename DestinationIterator,
         typename ValueType>
BOOST_DECIMAL_GPU_ENABLED constexpr auto fill_unsafe(DestinationIterator first, DestinationIterator last, ValueType val) -> void
{
  while(first != last)
  {
    using local_destination_value_type = typename iterator_detail::iterator_traits<DestinationIterator>::value_type;

    *first++ = static_cast<local_destination_value_type>(val);
  }
}

// Use a local, constexpr, unsafe implementation of the copy-function.
template<typename InputIterator,
         typename DestinationIterator>
BOOST_DECIMAL_GPU_ENABLED constexpr auto copy_unsafe(InputIterator first, InputIterator last, DestinationIterator dest) -> DestinationIterator
{
  while(first != last)
  {
    using local_destination_value_type = typename iterator_detail::iterator_traits<DestinationIterator>::value_type;
    #if (defined(__GNUC__) && (__GNUC__ > 9))
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
    #endif
    *dest++ = static_cast<local_destination_value_type>(*first++);
    #if (defined(__GNUC__) && (__GNUC__ > 9))
    #pragma GCC diagnostic pop
    #endif
  }

  return dest;
}

namespace array_detail {

template<typename T, std::size_t N>
class array_unsafe
{
public:
  // Standard container-local type definitions.
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  using value_type             = T;
  using pointer                = T*;
  using const_pointer          = const T*;
  using reference              = T&;
  using const_reference        = const T&;
  using iterator               = pointer;
  using const_iterator         = const_pointer;
  using reverse_iterator       = iterator_detail::reverse_iterator<iterator>;
  using const_reverse_iterator = iterator_detail::reverse_iterator<const_iterator>;

  value_type elems[N] { }; // NOLINT(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays,misc-non-private-member-variables-in-classes)

  static constexpr size_type static_size = N;

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto begin() -> iterator { return elems; }                 // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto end  () -> iterator { return elems + N; }             // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto begin() const -> const_iterator { return elems; }     // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto end  () const -> const_iterator { return elems + N; } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto cbegin() const -> const_iterator { return elems; }     // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto cend  () const -> const_iterator { return elems + N; } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto rbegin() -> reverse_iterator { return reverse_iterator(elems + N); }                    // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto rend  () -> reverse_iterator { return reverse_iterator(elems); }                        // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto rbegin() const -> const_reverse_iterator { return const_reverse_iterator(elems + N); }  // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto rend  () const -> const_reverse_iterator { return const_reverse_iterator(elems); }      // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto crbegin() const -> const_reverse_iterator { return const_reverse_iterator(elems + N); } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto crend  () const -> const_reverse_iterator { return const_reverse_iterator(elems); }     // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto operator[](const size_type i)       -> reference       { return elems[i]; } // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto operator[](const size_type i) const -> const_reference { return elems[i]; } // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto at(const size_type i)       -> reference       { return elems[i]; } // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto at(const size_type i) const -> const_reference { return elems[i]; } // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto front()       -> reference       { return elems[static_cast<size_type>(UINT8_C(0))]; }
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto front() const -> const_reference { return elems[static_cast<size_type>(UINT8_C(0))]; }

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto back()       -> reference       { return elems[static_cast<size_type>(N - static_cast<size_type>(UINT8_C(1)))]; }
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto back() const -> const_reference { return elems[static_cast<size_type>(N - static_cast<size_type>(UINT8_C(1)))]; }

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD static BOOST_DECIMAL_GPU_ENABLED constexpr auto size()     -> size_type { return N; }
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD static BOOST_DECIMAL_GPU_ENABLED constexpr auto empty()    -> bool      { return false; }
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD static BOOST_DECIMAL_GPU_ENABLED constexpr auto max_size() -> size_type { return N; }

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto data() const -> const_pointer { return elems; } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto data()       -> pointer       { return elems; } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_GPU_ENABLED constexpr auto c_array() -> pointer { return elems; }

  template<typename T2>
  BOOST_DECIMAL_GPU_ENABLED constexpr auto operator=(const array_unsafe<T2, N>& y) -> array_unsafe&
  {
    copy_unsafe(y.begin(), y.end(), begin());

    return *this;
  }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto assign(const value_type& value) -> void
  {
    fill_unsafe(elems, elems + N, value); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  BOOST_DECIMAL_GPU_ENABLED constexpr auto fill(const value_type& value) -> void
  {
    assign(value);
  }
};

} // namespace array_detail

using size_t    = std::uint32_t;
using ptrdiff_t = std::int32_t;

static_assert((  (boost::decimal::detail::numeric_limits<size_t>::digits        >= boost::decimal::detail::numeric_limits<std::uint16_t>::digits)
              && (boost::decimal::detail::numeric_limits<ptrdiff_t>::digits + 1 >= boost::decimal::detail::numeric_limits<std::uint16_t>::digits)),
              "Error: size type and pointer difference type must be at least 16 bits in width (or wider)");

template<const size_t Width2> struct verify_power_of_two // NOLINT(altera-struct-pack-align)
{
  // TBD: Which powers should be checked if size_t is not 32 bits?
  static constexpr auto conditional_value =
       (Width2 == static_cast<size_t>(1ULL <<  0U)) || (Width2 == static_cast<size_t>(1ULL <<  1U)) || (Width2 == static_cast<size_t>(1ULL <<  2U)) || (Width2 == static_cast<size_t>(1ULL <<  3U))
    || (Width2 == static_cast<size_t>(1ULL <<  4U)) || (Width2 == static_cast<size_t>(1ULL <<  5U)) || (Width2 == static_cast<size_t>(1ULL <<  6U)) || (Width2 == static_cast<size_t>(1ULL <<  7U))
    || (Width2 == static_cast<size_t>(1ULL <<  8U)) || (Width2 == static_cast<size_t>(1ULL <<  9U)) || (Width2 == static_cast<size_t>(1ULL << 10U)) || (Width2 == static_cast<size_t>(1ULL << 11U))
    || (Width2 == static_cast<size_t>(1ULL << 12U)) || (Width2 == static_cast<size_t>(1ULL << 13U)) || (Width2 == static_cast<size_t>(1ULL << 14U)) || (Width2 == static_cast<size_t>(1ULL << 15U))
    || (Width2 == static_cast<size_t>(1ULL << 16U)) || (Width2 == static_cast<size_t>(1ULL << 17U)) || (Width2 == static_cast<size_t>(1ULL << 18U)) || (Width2 == static_cast<size_t>(1ULL << 19U))
    || (Width2 == static_cast<size_t>(1ULL << 20U)) || (Width2 == static_cast<size_t>(1ULL << 21U)) || (Width2 == static_cast<size_t>(1ULL << 22U)) || (Width2 == static_cast<size_t>(1ULL << 23U))
    || (Width2 == static_cast<size_t>(1ULL << 24U)) || (Width2 == static_cast<size_t>(1ULL << 25U)) || (Width2 == static_cast<size_t>(1ULL << 26U)) || (Width2 == static_cast<size_t>(1ULL << 27U))
    || (Width2 == static_cast<size_t>(1ULL << 28U)) || (Width2 == static_cast<size_t>(1ULL << 29U)) || (Width2 == static_cast<size_t>(1ULL << 30U)) || (Width2 == static_cast<size_t>(1ULL << 31U))
    ;
};

template<const size_t BitCount,
         typename EnableType = void>
struct uint_type_helper
{
private:
  static BOOST_DECIMAL_GPU_ENABLED constexpr auto bit_count   () -> size_t { return BitCount; }
  static BOOST_DECIMAL_GPU_ENABLED constexpr auto bit_count_lo() -> size_t { return static_cast<size_t>(UINT8_C(8)); }
  static BOOST_DECIMAL_GPU_ENABLED constexpr auto bit_count_hi() -> size_t { return static_cast<size_t>(UINT8_C(64)); }

  static_assert((   ((bit_count() >= bit_count_lo()) && (BitCount <= bit_count_hi())) // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                 && (verify_power_of_two<bit_count()>::conditional_value)),
                "Error: uint_type_helper is not intended to be used for this BitCount");

public:
  using exact_unsigned_type = std::uintmax_t;
  using exact_signed_type   = std::intmax_t;
  using fast_unsigned_type  = std::uintmax_t;
  using fast_signed_type    = std::intmax_t;
};

template<const size_t BitCount> struct uint_type_helper<BitCount, BOOST_DECIMAL_TYPE_TRAITS_NAMESPACE::enable_if_t<                                                  (BitCount <= static_cast<size_t>(UINT8_C(  8)))>> { using exact_unsigned_type = std::uint8_t;      using exact_signed_type = std::int8_t;     using fast_unsigned_type = std::uint_fast8_t;  using fast_signed_type = std::int_fast8_t;  };
template<const size_t BitCount> struct uint_type_helper<BitCount, BOOST_DECIMAL_TYPE_TRAITS_NAMESPACE::enable_if_t<(BitCount >= static_cast<size_t>(UINT8_C( 9))) && (BitCount <= static_cast<size_t>(UINT8_C( 16)))>> { using exact_unsigned_type = std::uint16_t;     using exact_signed_type = std::int16_t;    using fast_unsigned_type = std::uint_fast16_t; using fast_signed_type = std::int_fast16_t; };
template<const size_t BitCount> struct uint_type_helper<BitCount, BOOST_DECIMAL_TYPE_TRAITS_NAMESPACE::enable_if_t<(BitCount >= static_cast<size_t>(UINT8_C(17))) && (BitCount <= static_cast<size_t>(UINT8_C( 32)))>> { using exact_unsigned_type = std::uint32_t;     using exact_signed_type = std::int32_t;    using fast_unsigned_type = std::uint_fast32_t; using fast_signed_type = std::int_fast32_t; };
template<const size_t BitCount> struct uint_type_helper<BitCount, BOOST_DECIMAL_TYPE_TRAITS_NAMESPACE::enable_if_t<(BitCount >= static_cast<size_t>(UINT8_C(33))) && (BitCount <= static_cast<size_t>(UINT8_C( 64)))>> { using exact_unsigned_type = std::uint64_t;     using exact_signed_type = std::int64_t;    using fast_unsigned_type = std::uint_fast64_t; using fast_signed_type = std::int_fast64_t; };

using unsigned_fast_type = typename uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<size_t   >::digits + 0)>::fast_unsigned_type;
using   signed_fast_type = typename uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<ptrdiff_t>::digits + 1)>::fast_signed_type;

template<typename InputIterator,
         typename IntegralType>
BOOST_DECIMAL_GPU_ENABLED constexpr auto advance_and_point(InputIterator it, IntegralType n) -> InputIterator
{
  using local_signed_integral_type =
    BOOST_DECIMAL_TYPE_TRAITS_NAMESPACE::conditional_t<BOOST_DECIMAL_TYPE_TRAITS_NAMESPACE::is_signed<IntegralType>::value,
                       IntegralType,
                       typename detail::uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<IntegralType>::digits)>::exact_signed_type>;

  using local_difference_type = typename detail::iterator_detail::iterator_traits<InputIterator>::difference_type;

  return it + static_cast<local_difference_type>(static_cast<local_signed_integral_type>(n));
}

template<typename UnsignedShortType,
         typename UnsignedLargeType = typename detail::uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<UnsignedShortType>::digits * 2)>::exact_unsigned_type>
BOOST_DECIMAL_GPU_ENABLED constexpr auto make_lo(const UnsignedLargeType& u) -> UnsignedShortType
{
  // From an unsigned integral input parameter of type UnsignedLargeType,
  // extract the low part of it. The type of the extracted
  // low part is UnsignedShortType, which has half the width of UnsignedLargeType.

  using local_ushort_type = UnsignedShortType;
  using local_ularge_type = UnsignedLargeType;

  // Compile-time checks.
  static_assert((    ( boost::decimal::detail::numeric_limits<local_ushort_type>::is_integer)
                 &&  ( boost::decimal::detail::numeric_limits<local_ularge_type>::is_integer)
                 &&  (!boost::decimal::detail::numeric_limits<local_ushort_type>::is_signed)
                 &&  (!boost::decimal::detail::numeric_limits<local_ularge_type>::is_signed)
                 &&  ((sizeof(local_ushort_type) * 2U) == sizeof(local_ularge_type))),
                 "Error: Please check the characteristics of the template parameters UnsignedShortType and UnsignedLargeType");

  return static_cast<local_ushort_type>(u);
}

template<typename UnsignedShortType,
         typename UnsignedLargeType = typename detail::uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<UnsignedShortType>::digits * 2)>::exact_unsigned_type>
BOOST_DECIMAL_GPU_ENABLED constexpr auto make_hi(const UnsignedLargeType& u) -> UnsignedShortType
{
  // From an unsigned integral input parameter of type UnsignedLargeType,
  // extract the high part of it. The type of the extracted
  // high part is UnsignedShortType, which has half the width of UnsignedLargeType.

  using local_ushort_type = UnsignedShortType;
  using local_ularge_type = UnsignedLargeType;

  // Compile-time checks.
  static_assert((    ( boost::decimal::detail::numeric_limits<local_ushort_type>::is_integer)
                 &&  ( boost::decimal::detail::numeric_limits<local_ularge_type>::is_integer)
                 &&  (!boost::decimal::detail::numeric_limits<local_ushort_type>::is_signed)
                 &&  (!boost::decimal::detail::numeric_limits<local_ularge_type>::is_signed)
                 &&  ((sizeof(local_ushort_type) * 2U) == sizeof(local_ularge_type))),
                 "Error: Please check the characteristics of the template parameters UnsignedShortType and UnsignedLargeType");

  return static_cast<local_ushort_type>(u >> static_cast<local_ushort_type>(boost::decimal::detail::numeric_limits<local_ushort_type>::digits));
}

template<typename UnsignedShortType,
         typename UnsignedLargeType = typename detail::uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<UnsignedShortType>::digits * 2)>::exact_unsigned_type>
BOOST_DECIMAL_GPU_ENABLED constexpr auto make_large(const UnsignedShortType& lo, const UnsignedShortType& hi) -> UnsignedLargeType
{
  // Create a composite unsigned integral value having type UnsignedLargeType.
  // Two constituents are used having type UnsignedShortType, whereby the
  // width of UnsignedShortType is half the width of UnsignedLargeType.

  using local_ushort_type = UnsignedShortType;
  using local_ularge_type = UnsignedLargeType;

  // Compile-time checks.
  static_assert((    ( boost::decimal::detail::numeric_limits<local_ushort_type>::is_integer)
                 &&  ( boost::decimal::detail::numeric_limits<local_ularge_type>::is_integer)
                 &&  (!boost::decimal::detail::numeric_limits<local_ushort_type>::is_signed)
                 &&  (!boost::decimal::detail::numeric_limits<local_ularge_type>::is_signed)
                 &&  ((sizeof(local_ushort_type) * 2U) == sizeof(local_ularge_type))),
                 "Error: Please check the characteristics of the template parameters UnsignedShortType and UnsignedLargeType");

  return
    static_cast<local_ularge_type>
    (
        static_cast<local_ularge_type>
        (
          static_cast<local_ularge_type>(hi) << static_cast<unsigned>(boost::decimal::detail::numeric_limits<UnsignedShortType>::digits)
        )
      | lo
    );
}

} // namespace detail

using detail::size_t;
using detail::ptrdiff_t;
using detail::unsigned_fast_type;
using detail::signed_fast_type;

template<const size_t Width2>
class uintwide_t
{
public:
  // Class-local type definitions.
  using limb_type = std::uint32_t;

  using double_limb_type =
    typename detail::uint_type_helper<static_cast<size_t>(static_cast<int>(boost::decimal::detail::numeric_limits<limb_type>::digits * static_cast<int>(INT8_C(2))))>::exact_unsigned_type;

  // Legacy ularge and ushort types. These are no longer used
  // in the class, but provided for legacy compatibility.
  using ushort_type = limb_type;
  using ularge_type = double_limb_type;

  // More compile-time checks.
  static_assert((    ( boost::decimal::detail::numeric_limits<limb_type>::is_integer)
                 &&  ( boost::decimal::detail::numeric_limits<double_limb_type>::is_integer)
                 &&  (!boost::decimal::detail::numeric_limits<limb_type>::is_signed)
                 &&  (!boost::decimal::detail::numeric_limits<double_limb_type>::is_signed)
                 &&  ((sizeof(limb_type) * 2U) == sizeof(double_limb_type))),
                 "Error: Please check the characteristics of the template parameters UnsignedShortType and UnsignedLargeType");

  // Helper constants for the digit characteristics.
  static constexpr size_t my_width2 = Width2;

  // The number of limbs.
  static constexpr size_t number_of_limbs =
    static_cast<size_t>
    (
      Width2 / static_cast<size_t>(boost::decimal::detail::numeric_limits<limb_type>::digits)
    );

  // The type of the internal data representation.
  using representation_type = detail::array_detail::array_unsafe<limb_type, static_cast<std::size_t>(number_of_limbs)>;

  // The iterator types of the internal data representation.
  using iterator               = typename representation_type::iterator;
  using const_iterator         = typename representation_type::const_iterator;
  using reverse_iterator       = typename representation_type::reverse_iterator;
  using const_reverse_iterator = typename representation_type::const_reverse_iterator;

  // Provide a user interface to the internal data representation.
  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD
  BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR 
  BOOST_DECIMAL_GPU_ENABLED auto  representation()       ->       representation_type& { return values; }

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD
  BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR 
  BOOST_DECIMAL_GPU_ENABLED auto crepresentation() const -> const representation_type& { return values; }

  BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR BOOST_DECIMAL_GPU_ENABLED auto eval_divide_knuth(const uintwide_t& other, uintwide_t& remainder) -> void
  {
    using local_uint_index_type = unsigned_fast_type;

    local_uint_index_type u_offset { };
    local_uint_index_type v_offset { };

    auto crit_u =       values.crbegin();
    auto crit_v = other.values.crbegin();

    while(crit_u != values.crend() && (*crit_u == static_cast<limb_type>(UINT8_C(0)))) // NOLINT(altera-id-dependent-backward-branch)
    {
      ++crit_u;
      ++u_offset;
    }

    while(crit_v != other.values.crend() && (*crit_v == static_cast<limb_type>(UINT8_C(0)))) // NOLINT(altera-id-dependent-backward-branch)
    {
      ++crit_v;
      ++v_offset;
    }

    if(v_offset == static_cast<local_uint_index_type>(number_of_limbs))
    {
      // The denominator is zero. Set to zero and return.
      detail::fill_unsafe(values.begin(), values.end(), static_cast<limb_type>(UINT8_C(0)));

      detail::fill_unsafe(remainder.values.begin(), remainder.values.end(), static_cast<limb_type>(UINT8_C(0)));
    }
    else if(u_offset == static_cast<local_uint_index_type>(number_of_limbs))
    {
      // The numerator is zero.
      // Do nothing other than set the remainder to zero and return.
      detail::fill_unsafe(remainder.values.begin(), remainder.values.end(), static_cast<limb_type>(UINT8_C(0)));
    }
    else
    {
      const auto result_of_compare_left_with_right = compare(other);

      const auto left_is_less_than_right = (result_of_compare_left_with_right == INT8_C(-1));
      const auto left_is_equal_to_right  = (result_of_compare_left_with_right == INT8_C( 0));

      if(left_is_less_than_right)
      {
        // If the denominator is larger than the numerator,
        // then the result of the division is zero.
        remainder = *this;

        // Set the result to zero.
        detail::fill_unsafe(values.begin(), values.end(), static_cast<limb_type>(UINT8_C(0)));
      }
      else if(left_is_equal_to_right)
      {
        // If the denominator is equal to the numerator,
        // then the result of the division is one.

        // Set the result to one.
        detail::fill_unsafe(values.begin() + static_cast<size_t>(UINT8_C(1)), values.end(), static_cast<limb_type>(UINT8_C(0)));
        values.front() = static_cast<limb_type>(UINT8_C(1));

        // Set the remainder to zero.
        detail::fill_unsafe(remainder.values.begin(), remainder.values.end(), static_cast<limb_type>(UINT8_C(0)));
      }
      else
      {
        eval_divide_knuth_core(u_offset, v_offset, other, remainder);
      }
    }
  }

  template<typename ResultIterator,
           typename InputIteratorLeft>
  BOOST_DECIMAL_GPU_ENABLED
  static BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR auto eval_multiply_1d(      ResultIterator                                                                   r,
                                                                          InputIteratorLeft                                                                a,
                                                                    const typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type b,
                                                                    const unsigned_fast_type                                                               count) -> limb_type
  {
    using local_limb_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::value_type;
    using left_value_type = typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type;

    static_assert
    (
      (boost::decimal::detail::numeric_limits<local_limb_type>::digits == boost::decimal::detail::numeric_limits<left_value_type>::digits),
      "Error: Internals require same widths for left-right-result limb_types at the moment"
    );

    using local_double_limb_type =
      typename detail::uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<local_limb_type>::digits * 2)>::exact_unsigned_type;

    auto carry = static_cast<local_double_limb_type>(UINT8_C(0));

    if(b == static_cast<left_value_type>(UINT8_C(0)))
    {
      detail::fill_unsafe(r, detail::advance_and_point(r, count), static_cast<limb_type>(UINT8_C(0)));
    }
    else
    {
      const auto imax = count;

      auto i = static_cast<unsigned_fast_type>(UINT8_C(0));

      for( ; i < imax; ++i) // NOLINT(altera-id-dependent-backward-branch)
      {
        carry =
          static_cast<local_double_limb_type>
          (
              carry
            + static_cast<local_double_limb_type>(static_cast<local_double_limb_type>(*a++) * b)
          );

        *r++  = static_cast<local_limb_type>(carry);
        carry = detail::make_hi<local_limb_type>(carry);
      }
    }

    return static_cast<local_limb_type>(carry);
  }

  template<typename ResultIterator,
           typename InputIteratorLeft,
           typename InputIteratorRight>
  BOOST_DECIMAL_GPU_ENABLED
  static constexpr auto eval_multiply_n_by_n_to_lo_part_128(      ResultIterator     r,
                                                                  InputIteratorLeft  a,
                                                                  InputIteratorRight b,
                                                            const unsigned_fast_type count) -> void
  {
    static_cast<void>(count);

    using local_limb_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::value_type;

    static_assert
    (
         (boost::decimal::detail::numeric_limits<local_limb_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type>::digits)
      && (boost::decimal::detail::numeric_limits<local_limb_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorRight>::value_type>::digits),
      "Error: Internals require same widths for left-right-result limb_types at the moment"
    );

    using local_double_limb_type =
      typename detail::uint_type_helper<static_cast<size_t>(static_cast<int>(boost::decimal::detail::numeric_limits<local_limb_type>::digits * static_cast<int>(INT8_C(2))))>::exact_unsigned_type;

    using result_difference_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::difference_type;
    using left_difference_type   = typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::difference_type;
    using left_value_type        = typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type;
    using right_difference_type  = typename detail::iterator_detail::iterator_traits<InputIteratorRight>::difference_type;

    // The algorithm has been derived from the polynomial multiplication.
    // After the multiplication terms of equal order are grouped
    // together and retained up to order(3). The carries from the
    // multiplications are included when adding up the terms.
    // The results of the intermediate multiplications are stored
    // in local variables in memory.

    //   Column[CoefficientList[Expand[(a0 + a1 x + a2 x^2 + a3 x^3) (b0 + b1 x + b2 x^2 + b3 x^3)], x]]
    //   a0b0
    //   a1b0 + a0b1
    //   a2b0 + a1b1 + a0b2
    //   a3b0 + a2b1 + a1b2 + a0b3

    // See also Wolfram Alpha at:
    // https://www.wolframalpha.com/input/?i=Column%5BCoefficientList%5B+++Expand%5B%28a0+%2B+a1+x+%2B+a2+x%5E2+%2B+a3+x%5E3%29+%28b0+%2B+b1+x+%2B+b2+x%5E2+%2B+b3+x%5E3%29%5D%2C++++x%5D%5D
    // ... and take the upper half of the pyramid.

    // Performance improvement:
    //   (old) kops_per_sec: 33173.50
    //   (new) kops_per_sec: 95069.43

    local_double_limb_type r1 { };
    local_double_limb_type r2 { };

    const auto a0b0 = static_cast<local_double_limb_type>(*detail::advance_and_point(a, static_cast<left_difference_type>(0)) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0)))));
    const auto a0b1 = static_cast<local_double_limb_type>(*detail::advance_and_point(a, static_cast<left_difference_type>(0)) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1)))));
    const auto a1b0 = static_cast<local_double_limb_type>(*detail::advance_and_point(a, static_cast<left_difference_type>(1)) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0)))));
    const auto a1b1 = static_cast<local_double_limb_type>(*detail::advance_and_point(a, static_cast<left_difference_type>(1)) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1)))));

    // One special case is considered, the case of multiplication
    // of the form BITS/2 * BITS/2 = BITS. In this case, the algorithm
    // can be significantly simplified by using only the 'lower-halves'
    // of the data.
    if(    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) == static_cast<left_value_type>(UINT8_C(0))) && (*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2))) == static_cast<left_value_type>(UINT8_C(0)))
        && (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(3))) == static_cast<left_value_type>(UINT8_C(0))) && (*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(3))) == static_cast<left_value_type>(UINT8_C(0))))
    {
      r1    = static_cast<local_double_limb_type>
              (
                static_cast<local_double_limb_type>
                (
                  detail::make_hi<local_limb_type>(a0b0) // LCOV_EXCL_LINE
                )
                + detail::make_lo<local_limb_type>(a1b0)
                + detail::make_lo<local_limb_type>(a0b1)
              )
              ;
      r2    = static_cast<local_double_limb_type>
              (
                static_cast<local_double_limb_type>
                (
                  detail::make_hi<local_limb_type>(r1) // LCOV_EXCL_LINE
                )
                + detail::make_lo<local_limb_type>(a1b1)
                + detail::make_hi<local_limb_type>(a0b1)
                + detail::make_hi<local_limb_type>(a1b0)
              )
              ;
      *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(3)))
            = static_cast<local_limb_type>
              (
                  detail::make_hi<local_limb_type>(r2)
                + detail::make_hi<local_limb_type>(a1b1)
              )
              ;
    }
    else
    {
      const auto a0b2 = static_cast<local_double_limb_type>(*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2)))));
      const auto a2b0 = static_cast<local_double_limb_type>(*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0)))));

      r1    = static_cast<local_double_limb_type>
              (
                static_cast<local_double_limb_type>
                (
                  detail::make_hi<local_limb_type>(a0b0)
                )
                + detail::make_lo<local_limb_type>(a1b0)
                + detail::make_lo<local_limb_type>(a0b1)
              )
              ;
      r2    = static_cast<local_double_limb_type>
              (
                static_cast<local_double_limb_type>
                (
                  detail::make_hi<local_limb_type>(r1)
                )
                + detail::make_lo<local_limb_type>(a2b0)
                + detail::make_lo<local_limb_type>(a1b1)
                + detail::make_lo<local_limb_type>(a0b2)
                + detail::make_hi<local_limb_type>(a1b0)
                + detail::make_hi<local_limb_type>(a0b1)
              )
              ;
      *detail::advance_and_point(r, static_cast<result_difference_type>(3))
            = static_cast<local_limb_type>
              (
                  detail::make_hi<local_limb_type>(r2)
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(3))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(3)))))
                + detail::make_hi<local_limb_type>(a2b0)
                + detail::make_hi<local_limb_type>(a1b1)
                + detail::make_hi<local_limb_type>(a0b2)
              )
              ;
    }

    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(0))) = static_cast<local_limb_type>(a0b0);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(1))) = static_cast<local_limb_type>(r1);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(2))) = static_cast<local_limb_type>(r2);
  }

  template<typename ResultIterator,
           typename InputIteratorLeft,
           typename InputIteratorRight>
  BOOST_DECIMAL_GPU_ENABLED
  static BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR auto eval_multiply_n_by_n_to_lo_part_256(      ResultIterator     r,
                                                                                             InputIteratorLeft  a,
                                                                                             InputIteratorRight b,
                                                                                       const unsigned_fast_type count) -> void
  {
    static_cast<void>(count);

    static_assert
    (
          (boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<ResultIterator>::value_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type>::digits)
      && (boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<ResultIterator>::value_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorRight>::value_type>::digits),
      "Error: Internals require same widths for left-right-result limb_types at the moment"
    );

    using local_limb_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::value_type;

    using local_double_limb_type =
      typename detail::uint_type_helper<static_cast<size_t>(static_cast<int>(boost::decimal::detail::numeric_limits<local_limb_type>::digits * static_cast<int>(INT8_C(2))))>::exact_unsigned_type;

    using result_difference_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::difference_type;
    using left_difference_type   = typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::difference_type;
    using left_value_type        = typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type;
    using right_difference_type  = typename detail::iterator_detail::iterator_traits<InputIteratorRight>::difference_type;

    // The algorithm has been derived from the polynomial multiplication.
    // After the multiplication terms of equal order are grouped
    // together and retained up to order(3). The carries from the
    // multiplications are included when adding up the terms.
    // The results of the intermediate multiplications are stored
    // in local variables in memory.

    //   Column[CoefficientList[Expand[(a0 + a1 x + a2 x^2 + a3 x^3 + a4 x^4 + a5 x^5 + a6 x^6 + a7 x^7) (b0 + b1 x + b2 x^2 + b3 x^3 + b4 x^4 + b5 x^5 + b6 x^6 + b7 x^7)], x]]
    //   a0b0
    //   a1b0 + a0b1
    //   a2b0 + a1b1 + a0b2
    //   a3b0 + a2b1 + a1b2 + a0b3
    //   a4b0 + a3b1 + a2b2 + a1b3 + a0b4
    //   a5b0 + a4b1 + a3b2 + a2b3 + a1b4 + a0b5
    //   a6b0 + a5b1 + a4b2 + a3b3 + a2b4 + a1b5 + a0b6
    //   a7b0 + a6b1 + a5b2 + a4b3 + a3b4 + a2b5 + a1b6 + a0b7

    // See also Wolfram Alpha at:
    // https://www.wolframalpha.com/input/?i=Column%5BCoefficientList%5B+++Expand%5B%28a0+%2B+a1+x+%2B+a2+x%5E2+%2B+a3+x%5E3%29+%28b0+%2B+b1+x+%2B+b2+x%5E2+%2B+b3+x%5E3%29%5D%2C++++x%5D%5D
    // ... and take the upper half of the pyramid.

    const local_double_limb_type a0b0 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0))));

    const local_double_limb_type a1b0 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0))));
    const local_double_limb_type a0b1 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1))));

    const local_double_limb_type a2b0 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0))));
    const local_double_limb_type a1b1 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1))));
    const local_double_limb_type a0b2 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2))));

    const local_double_limb_type a3b0 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(3))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0))));
    const local_double_limb_type a2b1 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1))));
    const local_double_limb_type a1b2 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2))));
    const local_double_limb_type a0b3 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(3))));

    const local_double_limb_type a3b1 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(3))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1))));
    const local_double_limb_type a2b2 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2))));
    const local_double_limb_type a1b3 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(3))));

    const local_double_limb_type a3b2 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(3))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2))));
    const local_double_limb_type a2b3 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(3))));

    const local_double_limb_type a3b3 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(3))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(3))));

          local_double_limb_type rd1 { };
          local_double_limb_type rd2 { };
          local_double_limb_type rd3 { };
          local_double_limb_type rd4 { };
          local_double_limb_type rd5 { };
          local_double_limb_type rd6 { };

    // One special case is considered, the case of multiplication
    // of the form BITS/2 * BITS/2 = BITS. In this case, the algorithm
    // can be significantly simplified by using only the 'lower-halves'
    // of the data.
    if(    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(7))) == static_cast<left_value_type>(UINT8_C(0))) && (*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(7))) == static_cast<left_value_type>(UINT8_C(0)))
        && (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(6))) == static_cast<left_value_type>(UINT8_C(0))) && (*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(6))) == static_cast<left_value_type>(UINT8_C(0)))
        && (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(5))) == static_cast<left_value_type>(UINT8_C(0))) && (*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(5))) == static_cast<left_value_type>(UINT8_C(0)))
        && (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(4))) == static_cast<left_value_type>(UINT8_C(0))) && (*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(4))) == static_cast<left_value_type>(UINT8_C(0))))
    {
      rd1   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(a0b0)
              )
              + detail::make_lo<local_limb_type>(a1b0)
              + detail::make_lo<local_limb_type>(a0b1)
              ;

      rd2   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd1)
              )
              + detail::make_lo<local_limb_type>(a2b0)
              + detail::make_lo<local_limb_type>(a1b1)
              + detail::make_lo<local_limb_type>(a0b2)
              + detail::make_hi<local_limb_type>(a1b0)
              + detail::make_hi<local_limb_type>(a0b1)
              ;

      rd3   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd2)
              )
              + detail::make_lo<local_limb_type>(a3b0)
              + detail::make_lo<local_limb_type>(a2b1)
              + detail::make_lo<local_limb_type>(a1b2)
              + detail::make_lo<local_limb_type>(a0b3)
              + detail::make_hi<local_limb_type>(a2b0)
              + detail::make_hi<local_limb_type>(a1b1)
              + detail::make_hi<local_limb_type>(a0b2)
              ;

      rd4   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd3)
              )
              + detail::make_lo<local_limb_type>(a3b1)
              + detail::make_lo<local_limb_type>(a2b2)
              + detail::make_lo<local_limb_type>(a1b3)
              + detail::make_hi<local_limb_type>(a3b0)
              + detail::make_hi<local_limb_type>(a2b1)
              + detail::make_hi<local_limb_type>(a1b2)
              + detail::make_hi<local_limb_type>(a0b3)
              ;

      rd5   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd4)
              )
              + detail::make_lo<local_limb_type>(a3b2)
              + detail::make_lo<local_limb_type>(a2b3)
              + detail::make_hi<local_limb_type>(a3b1)
              + detail::make_hi<local_limb_type>(a2b2)
              + detail::make_hi<local_limb_type>(a1b3)
              ;

      rd6   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd5)
              )
              + detail::make_lo<local_limb_type>(a3b3)
              + detail::make_hi<local_limb_type>(a3b2)
              + detail::make_hi<local_limb_type>(a2b3)
              ;

      *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(7)))
            = static_cast<local_limb_type>
              (
                  detail::make_hi<local_limb_type>(rd6)
                + detail::make_hi<local_limb_type>(a3b3)
              )
              ;
    }
    else
    {
      const local_double_limb_type a4b0 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(4))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0))));
      const local_double_limb_type a0b4 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(4))));

      const local_double_limb_type a5b0 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(5))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0))));
      const local_double_limb_type a4b1 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(4))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1))));

      const local_double_limb_type a1b4 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(4))));
      const local_double_limb_type a0b5 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(5))));

      const local_double_limb_type a6b0 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(6))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0))));
      const local_double_limb_type a5b1 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(5))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1))));

      const local_double_limb_type a4b2 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(4))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2))));
      const local_double_limb_type a2b4 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(4))));

      const local_double_limb_type a1b5 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(5))));
      const local_double_limb_type a0b6 = *detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(6))));

      rd1   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(a0b0)
              )
              + detail::make_lo<local_limb_type>(a1b0)
              + detail::make_lo<local_limb_type>(a0b1)
              ;

      rd2   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd1)
              )
              + detail::make_lo<local_limb_type>(a2b0)
              + detail::make_lo<local_limb_type>(a1b1)
              + detail::make_lo<local_limb_type>(a0b2)
              + detail::make_hi<local_limb_type>(a1b0)
              + detail::make_hi<local_limb_type>(a0b1)
              ;

      rd3   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd2)
              )
              + detail::make_lo<local_limb_type>(a3b0)
              + detail::make_lo<local_limb_type>(a2b1)
              + detail::make_lo<local_limb_type>(a1b2)
              + detail::make_lo<local_limb_type>(a0b3)
              + detail::make_hi<local_limb_type>(a2b0)
              + detail::make_hi<local_limb_type>(a1b1)
              + detail::make_hi<local_limb_type>(a0b2)
              ;

      rd4   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd3)
              )
              + detail::make_lo<local_limb_type>(a4b0)
              + detail::make_lo<local_limb_type>(a3b1)
              + detail::make_lo<local_limb_type>(a2b2)
              + detail::make_lo<local_limb_type>(a1b3)
              + detail::make_lo<local_limb_type>(a0b4)
              + detail::make_hi<local_limb_type>(a3b0)
              + detail::make_hi<local_limb_type>(a2b1)
              + detail::make_hi<local_limb_type>(a1b2)
              + detail::make_hi<local_limb_type>(a0b3)
              ;

      rd5   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd4)
              )
              + detail::make_lo<local_limb_type>(a5b0)
              + detail::make_lo<local_limb_type>(a4b1)
              + detail::make_lo<local_limb_type>(a3b2)
              + detail::make_lo<local_limb_type>(a2b3)
              + detail::make_lo<local_limb_type>(a1b4)
              + detail::make_lo<local_limb_type>(a0b5)
              + detail::make_hi<local_limb_type>(a4b0)
              + detail::make_hi<local_limb_type>(a3b1)
              + detail::make_hi<local_limb_type>(a2b2)
              + detail::make_hi<local_limb_type>(a1b3)
              + detail::make_hi<local_limb_type>(a0b4)
              ;

      rd6   = static_cast<local_double_limb_type>
              (
                detail::make_hi<local_limb_type>(rd5)
              )
              + detail::make_lo<local_limb_type>(a6b0)
              + detail::make_lo<local_limb_type>(a5b1)
              + detail::make_lo<local_limb_type>(a4b2)
              + detail::make_lo<local_limb_type>(a3b3)
              + detail::make_lo<local_limb_type>(a2b4)
              + detail::make_lo<local_limb_type>(a1b5)
              + detail::make_lo<local_limb_type>(a0b6)
              + detail::make_hi<local_limb_type>(a5b0)
              + detail::make_hi<local_limb_type>(a4b1)
              + detail::make_hi<local_limb_type>(a3b2)
              + detail::make_hi<local_limb_type>(a2b3)
              + detail::make_hi<local_limb_type>(a1b4)
              + detail::make_hi<local_limb_type>(a0b5)
              ;

      *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(7)))
            = static_cast<local_limb_type>
              (
                  detail::make_hi<local_limb_type>(rd6)
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(7))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(0)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(6))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(1)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(5))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(2)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(4))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(3)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(3))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(4)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(2))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(5)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(1))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(6)))))
                + static_cast<local_limb_type>    (*detail::advance_and_point(a, static_cast<left_difference_type>(INT8_C(0))) * static_cast<local_double_limb_type>(*detail::advance_and_point(b, static_cast<right_difference_type>(INT8_C(7)))))
                + detail::make_hi<local_limb_type>(a6b0)
                + detail::make_hi<local_limb_type>(a5b1)
                + detail::make_hi<local_limb_type>(a4b2)
                + detail::make_hi<local_limb_type>(a3b3)
                + detail::make_hi<local_limb_type>(a2b4)
                + detail::make_hi<local_limb_type>(a1b5)
                + detail::make_hi<local_limb_type>(a0b6)
              )
              ;
    }

    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(0))) = static_cast<local_limb_type>(a0b0);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(1))) = static_cast<local_limb_type>(rd1);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(2))) = static_cast<local_limb_type>(rd2);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(3))) = static_cast<local_limb_type>(rd3);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(4))) = static_cast<local_limb_type>(rd4);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(5))) = static_cast<local_limb_type>(rd5);
    *detail::advance_and_point(r, static_cast<result_difference_type>(INT8_C(6))) = static_cast<local_limb_type>(rd6);
  }

private:
  representation_type values { };

  BOOST_DECIMAL_WIDE_INTEGER_NODISCARD BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR BOOST_DECIMAL_GPU_ENABLED auto compare(const uintwide_t& other) const -> std::int_fast8_t
  {
    return compare_ranges(values.cbegin(), other.values.cbegin(), number_of_limbs);
  }

  template<typename InputIteratorLeftType,
           typename InputIteratorRightType>
  BOOST_DECIMAL_GPU_ENABLED
  static BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR auto compare_ranges(      InputIteratorLeftType  a,
                                                                        InputIteratorRightType b,
                                                                  const unsigned_fast_type     count) -> std::int_fast8_t
  {
    auto n_return = static_cast<std::int_fast8_t>(INT8_C(0));

    detail::iterator_detail::reverse_iterator<InputIteratorLeftType>  pa(detail::advance_and_point(a, count));
    detail::iterator_detail::reverse_iterator<InputIteratorRightType> pb(detail::advance_and_point(b, count));

    while(pa != detail::iterator_detail::reverse_iterator<InputIteratorLeftType>(a)) // NOLINT(altera-id-dependent-backward-branch)
    {
      using value_left_type = typename detail::iterator_detail::iterator_traits<InputIteratorLeftType>::value_type;

      const auto value_a = *pa++;
      const auto value_b = static_cast<value_left_type>(*pb++);

      if(value_a != value_b)
      {
        n_return =
          static_cast<std::int_fast8_t>
          (
            (value_a > value_b)
              ? static_cast<std::int_fast8_t>(INT8_C(1))
              : static_cast<std::int_fast8_t>(INT8_C(-1))
          );

        break;
      }
    }

    return n_return;
  }

  template<typename ResultIterator,
           typename InputIteratorLeft,
           typename InputIteratorRight>
  BOOST_DECIMAL_GPU_ENABLED
  static BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR auto eval_add_n(      ResultIterator     r,
                                                                    InputIteratorLeft  u,
                                                                    InputIteratorRight v,
                                                              const unsigned_fast_type count,
                                                              const limb_type          carry_in = static_cast<limb_type>(UINT8_C(0))) -> limb_type
  {
    auto carry_out = static_cast<std::uint_fast8_t>(carry_in);

    using local_limb_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::value_type;

    static_assert
    (
         (boost::decimal::detail::numeric_limits<local_limb_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type>::digits)
      && (boost::decimal::detail::numeric_limits<local_limb_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorRight>::value_type>::digits),
      "Error: Internals require same widths for left-right-result limb_types at the moment"
    );

    using local_double_limb_type =
      typename detail::uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<local_limb_type>::digits * 2)>::exact_unsigned_type;

    using result_difference_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::difference_type;

    for(auto i = static_cast<unsigned_fast_type>(UINT8_C(0)); i < count; ++i)
    {
      const auto uv_as_ularge =
        static_cast<local_double_limb_type>
        (
            static_cast<local_double_limb_type>(static_cast<local_double_limb_type>(*u++) + *v++)
          + carry_out
        );

      carry_out = static_cast<std::uint_fast8_t>(detail::make_hi<local_limb_type>(uv_as_ularge));

      *detail::advance_and_point(r, static_cast<result_difference_type>(i)) = static_cast<local_limb_type>(uv_as_ularge);
    }

    return static_cast<limb_type>(carry_out);
  }

  template<typename ResultIterator,
           typename InputIteratorLeft,
           typename InputIteratorRight>
  BOOST_DECIMAL_GPU_ENABLED
  static BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR auto eval_subtract_n(      ResultIterator     r,
                                                                         InputIteratorLeft  u,
                                                                         InputIteratorRight v,
                                                                   const unsigned_fast_type count,
                                                                   const bool               has_borrow_in = false) -> bool
  {
    auto has_borrow_out =
      static_cast<std::uint_fast8_t>
      (
        has_borrow_in ? static_cast<std::uint_fast8_t>(UINT8_C(1))
                      : static_cast<std::uint_fast8_t>(UINT8_C(0))
      );

    using local_limb_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::value_type;

    static_assert
    (
         (boost::decimal::detail::numeric_limits<local_limb_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorLeft>::value_type>::digits)
      && (boost::decimal::detail::numeric_limits<local_limb_type>::digits == boost::decimal::detail::numeric_limits<typename detail::iterator_detail::iterator_traits<InputIteratorRight>::value_type>::digits),
      "Error: Internals require same widths for left-right-result limb_types at the moment"
    );

    using local_double_limb_type =
      typename detail::uint_type_helper<static_cast<size_t>(boost::decimal::detail::numeric_limits<local_limb_type>::digits * 2)>::exact_unsigned_type;

    using result_difference_type = typename detail::iterator_detail::iterator_traits<ResultIterator>::difference_type;

    for(auto i = static_cast<unsigned_fast_type>(UINT8_C(0)); i < count; ++i)
    {
      const auto uv_as_ularge =
        static_cast<local_double_limb_type>
        (
            static_cast<local_double_limb_type>(static_cast<local_double_limb_type>(*u++) - *v++)
          - has_borrow_out
        );

      has_borrow_out =
        static_cast<std::uint_fast8_t>
        (
          (detail::make_hi<local_limb_type>(uv_as_ularge) != static_cast<local_limb_type>(UINT8_C(0)))
            ? static_cast<std::uint_fast8_t>(UINT8_C(1))
            : static_cast<std::uint_fast8_t>(UINT8_C(0))
        );

      *detail::advance_and_point(r, static_cast<result_difference_type>(i)) = static_cast<local_limb_type>(uv_as_ularge);
    }

    return (has_borrow_out != static_cast<std::uint_fast8_t>(UINT8_C(0)));
  }

  BOOST_DECIMAL_GPU_ENABLED
  BOOST_DECIMAL_WIDE_INTEGER_CONSTEXPR auto eval_divide_knuth_core(const unsigned_fast_type u_offset, // NOLINT(readability-function-cognitive-complexity)
                                                                   const unsigned_fast_type v_offset,
                                                                   const uintwide_t& other,
                                                                         uintwide_t& remainder) -> void
  {
    // Use Knuth's long division algorithm.
    // The loop-ordering of indices in Knuth's original
    // algorithm has been reversed due to the data format
    // used here. Several optimizations and combinations
    // of logic have been carried out in the source code.

    // See also:
    // D.E. Knuth, "The Art of Computer Programming, Volume 2:
    // Seminumerical Algorithms", Addison-Wesley (1998),
    // Section 4.3.1 Algorithm D and Exercise 16.

    using local_uint_index_type = unsigned_fast_type;

    // Compute the normalization factor d.
    const auto d =
      static_cast<limb_type>
      (
          static_cast<double_limb_type>(static_cast<double_limb_type>(UINT8_C(1)) << static_cast<unsigned>(boost::decimal::detail::numeric_limits<limb_type>::digits))
        / static_cast<double_limb_type>(static_cast<double_limb_type>(*detail::advance_and_point(other.values.cbegin(), static_cast<size_t>(static_cast<local_uint_index_type>(number_of_limbs - 1U) - v_offset))) + static_cast<limb_type>(1U))
      );

    // Step D1(b), normalize u -> u * d = uu.
    // Step D1(c): normalize v -> v * d = vv.

    using uu_array_type = detail::array_detail::array_unsafe<limb_type, static_cast<size_t>(number_of_limbs + static_cast<size_t>(UINT8_C(1)))>;

    uu_array_type uu { };

    representation_type vv { };

    if(d > static_cast<limb_type>(UINT8_C(1)))
    {
      {
        const auto num_limbs_minus_u_ofs =
          static_cast<size_t>
          (
            static_cast<local_uint_index_type>(number_of_limbs) - u_offset
          );

        const auto u_carry =
          eval_multiply_1d
          (
            uu.begin(),
            values.cbegin(),
            d,
            static_cast<unsigned_fast_type>(num_limbs_minus_u_ofs)
          );

        *(uu.begin() + num_limbs_minus_u_ofs) = u_carry;
      }

      static_cast<void>
      (
        eval_multiply_1d
        (
          vv.begin(),
          other.values.cbegin(),
          d,
          static_cast<unsigned_fast_type>(number_of_limbs - v_offset)
        )
      );
    }
    else
    {
      detail::copy_unsafe(values.cbegin(), values.cend(), uu.begin());

      *(uu.begin() + static_cast<size_t>(static_cast<local_uint_index_type>(number_of_limbs) - u_offset)) = static_cast<limb_type>(UINT8_C(0));

      vv = other.values;
    }

    // Step D2: Initialize j.
    // Step D7: Loop on j from m to 0.

    const auto n   = static_cast<local_uint_index_type>                                   (number_of_limbs - v_offset);
    const auto m   = static_cast<local_uint_index_type>(static_cast<local_uint_index_type>(number_of_limbs - u_offset) - n);
    const auto vj0 = static_cast<local_uint_index_type>(static_cast<local_uint_index_type>(n - static_cast<local_uint_index_type>(UINT8_C(1))));

    auto vv_at_vj0_it = detail::advance_and_point(vv.cbegin(), static_cast<size_t>(vj0)); // NOLINT(llvm-qualified-auto,readability-qualified-auto)

    const auto vv_at_vj0           = *vv_at_vj0_it--;
    const auto vv_at_vj0_minus_one = *vv_at_vj0_it;

    auto values_at_m_minus_j_it = detail::advance_and_point(values.begin(), static_cast<size_t>(m)); // NOLINT(llvm-qualified-auto,readability-qualified-auto)

    for(auto j = static_cast<local_uint_index_type>(UINT8_C(0)); j <= m; ++j) // NOLINT(altera-id-dependent-backward-branch)
    {
      // Step D3 [Calculate q_hat].
      //   if u[j] == v[j0]
      //     set q_hat = b - 1
      //   else
      //     set q_hat = (u[j] * b + u[j + 1]) / v[1]

      const auto uj     = static_cast<local_uint_index_type>(static_cast<local_uint_index_type>(static_cast<local_uint_index_type>(static_cast<local_uint_index_type>(number_of_limbs + 1U) - 1U) - u_offset) - j);
      const auto u_j_j1 = static_cast<double_limb_type>(static_cast<double_limb_type>(static_cast<double_limb_type>(*(uu.cbegin() + static_cast<size_t>(uj))) << static_cast<unsigned>(boost::decimal::detail::numeric_limits<limb_type>::digits)) + *(uu.cbegin() + static_cast<size_t>(uj - 1U)));

      auto q_hat =
        static_cast<limb_type>
        (
          (*(uu.cbegin() + static_cast<size_t>(uj)) == vv_at_vj0)
            ? (boost::decimal::detail::numeric_limits<limb_type>::max)()
            : static_cast<limb_type>(u_j_j1 / vv_at_vj0)
        );

      // Decrease q_hat if necessary.
      // This means that q_hat must be decreased if the
      // expression [(u[uj] * b + u[uj - 1] - q_hat * v[vj0 - 1]) * b]
      // exceeds the range of uintwide_t.

      for(auto t = static_cast<double_limb_type>(u_j_j1 - static_cast<double_limb_type>(q_hat * static_cast<double_limb_type>(vv_at_vj0)));
                  ;
                    --q_hat, t = static_cast<double_limb_type>(t + vv_at_vj0))
      {
        if(   (detail::make_hi<limb_type>(t) != static_cast<limb_type>(UINT8_C(0)))
            || (   static_cast<double_limb_type>(static_cast<double_limb_type>(vv_at_vj0_minus_one) * q_hat)
                <= static_cast<double_limb_type>(static_cast<double_limb_type>(t << static_cast<unsigned>(boost::decimal::detail::numeric_limits<limb_type>::digits)) + *detail::advance_and_point(uu.cbegin(), static_cast<size_t>(uj - 2U)))))
        {
          break;
        }
      }

      // Step D4: Multiply and subtract.
      // Replace u[j, ... j + n] by u[j, ... j + n] - q_hat * v[1, ... n].

      // Set nv = q_hat * (v[1, ... n]).
      uu_array_type nv { };

      *(nv.begin() + static_cast<size_t>(n)) = eval_multiply_1d(nv.begin(), vv.cbegin(), q_hat, n);

      const auto has_borrow =
        eval_subtract_n
        (
          detail::advance_and_point(uu.begin(),  static_cast<size_t>(static_cast<local_uint_index_type>(uj - n))),
          detail::advance_and_point(uu.cbegin(), static_cast<size_t>(static_cast<local_uint_index_type>(uj - n))),
          nv.cbegin(),
          static_cast<unsigned_fast_type>
          (
            static_cast<local_uint_index_type>(n + static_cast<local_uint_index_type>(UINT8_C(1)))
          )
        );

      // Step D5: Test the remainder.
      // Set the result value: Set result.m_data[m - j] = q_hat.
      // Use the condition (u[j] < 0), in other words if the borrow
      // is non-zero, then step D6 needs to be carried out.

      limb_type add_result { };

      if(has_borrow)
      {
        --q_hat;

        // Step D6: Add back.
        // Add v[1, ... n] back to u[j, ... j + n],
        // and decrease the result by 1.

        add_result =
          eval_add_n(uu.begin() + static_cast<size_t>(static_cast<local_uint_index_type>(uj - n)),
                     detail::advance_and_point(uu.cbegin(), static_cast<size_t>(static_cast<local_uint_index_type>(uj - n))),
                     vv.cbegin(),
                     static_cast<unsigned_fast_type>(n));
      }

      static_cast<void>(add_result);

      // Get the result data.
      *values_at_m_minus_j_it = static_cast<limb_type>(q_hat);

      if(j < m)
      {
        --values_at_m_minus_j_it;
      }
    }

    // Clear the data elements that have not
    // been computed in the division algorithm.
    {
      const auto m_plus_one =
        static_cast<local_uint_index_type>
        (
          static_cast<local_uint_index_type>(m) + static_cast<local_uint_index_type>(UINT8_C(1))
        );

      detail::fill_unsafe(detail::advance_and_point(values.begin(), m_plus_one), values.end(), static_cast<limb_type>(UINT8_C(0)));
    }

    {
      auto rl_it_fwd = // NOLINT(llvm-qualified-auto,readability-qualified-auto)
        detail::advance_and_point(remainder.values.begin(), static_cast<signed_fast_type>(n));

      if(d == static_cast<limb_type>(UINT8_C(1)))
      {
        detail::copy_unsafe(uu.cbegin(),
                            detail::advance_and_point(uu.cbegin(), static_cast<size_t>(static_cast<local_uint_index_type>(number_of_limbs - v_offset))),
                            remainder.values.begin());
      }
      else
      {
        auto previous_u = static_cast<limb_type>(UINT8_C(0));

        auto rl_it_rev = static_cast<reverse_iterator>(rl_it_fwd);

        auto ul =
          static_cast<signed_fast_type>
          (
            static_cast<size_t>
            (
                number_of_limbs
              - static_cast<size_t>(v_offset + static_cast<size_t>(UINT8_C(1)))
            )
          );

        for( ; rl_it_rev != remainder.values.rend(); ++rl_it_rev, --ul) // NOLINT(altera-id-dependent-backward-branch)
        {
          const auto t =
            static_cast<double_limb_type>
            (
                *(uu.cbegin() + static_cast<size_t>(ul))
              + static_cast<double_limb_type>
                (
                  static_cast<double_limb_type>(previous_u) << static_cast<unsigned>(boost::decimal::detail::numeric_limits<limb_type>::digits)
                )
            );

          *rl_it_rev = static_cast<limb_type>(static_cast<double_limb_type>(t / d));
          previous_u = static_cast<limb_type>(static_cast<double_limb_type>(t - static_cast<double_limb_type>(static_cast<double_limb_type>(d) * *rl_it_rev)));
        }
      }

      detail::fill_unsafe(rl_it_fwd, remainder.values.end(), static_cast<limb_type>(UINT8_C(0)));
    }
  }
};

// Define some convenient unsigned wide integer types.
using uint128_t = uintwide_t<static_cast<size_t>(UINT32_C(128))>;
using uint256_t = uintwide_t<static_cast<size_t>(UINT32_C(256))>;

} // namespace wide_integer
} // namespace math

#if defined(__clang__) && !defined(__GNUC__)
#pragma clang diagnostic pop
#endif

BOOST_DECIMAL_WIDE_INTEGER_NAMESPACE_END

#endif // UINTWIDE_T_2018_10_02_H
