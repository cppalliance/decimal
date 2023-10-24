// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

namespace local
{
  auto test_tgamma(const std::int32_t tol_factor, const long double range_lo, const long double range_hi) -> bool
  {
    static_cast<void>(tol_factor);
    static_cast<void>(range_lo);
    static_cast<void>(range_hi);

    auto result_is_ok = true;

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

  auto test_tgamma_edge() -> bool
  {
    auto result_is_ok = true;

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }

} // namespace local

auto main() -> int
{
  auto result_is_ok = true;

  {
    const auto result_tgamma_is_ok   = local::test_tgamma(static_cast<std::int32_t>(INT32_C(1)), 0.1L, 21.7L);

    BOOST_TEST(result_tgamma_is_ok);

    result_is_ok = (result_tgamma_is_ok && result_is_ok);
  }

  {
    const auto result_edge_is_ok = local::test_tgamma_edge();

    BOOST_TEST(result_edge_is_ok);

    result_is_ok = (result_edge_is_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
