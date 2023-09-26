// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#if 0
#if defined(__clang__)
  #if defined __has_feature
  #if __has_feature(thread_sanitizer)
  #define BOOST_DECIMAL_REDUCE_TEST_DEPTH
  #endif
  #endif
#elif defined(__GNUC__)
  #if defined(__SANITIZE_THREAD__)
  #define BOOST_DECIMAL_REDUCE_TEST_DEPTH
  #endif
#elif defined(_MSC_VER)
  #if defined(_DEBUG)
  #define BOOST_DECIMAL_REDUCE_TEST_DEPTH
  #endif
#endif
#endif

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

namespace local
{
  auto test_log1p() -> bool
  {
    auto result_is_ok = true;

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  auto test_log1p_edge() -> bool
  {
    auto result_is_ok = true;

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

} // namespace local

auto main() -> int
{
  auto result_is_ok = (local::test_log1p() && local::test_log1p_edge());

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
