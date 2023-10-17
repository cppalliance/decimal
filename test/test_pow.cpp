// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

namespace local
{
  template<typename DecimalType, typename FloatType>
  auto test_pow(const int tol_factor) -> bool
  {
    static_cast<void>(tol_factor);

    auto result_is_ok = true;

    BOOST_TEST(result_is_ok);

    return result_is_ok;
  }
}

auto main() -> int
{
  auto result_is_ok = true;

  {
    using decimal_type = boost::decimal::decimal32;
    using float_type   = float;

    const auto test_pow_is_ok = local::test_pow<decimal_type, float_type>(12);

    result_is_ok = (test_pow_is_ok && result_is_ok);
  }

  {
    using decimal_type = boost::decimal::decimal64;
    using float_type   = double;

    const auto test_pow_is_ok = local::test_pow<decimal_type, float_type>(12);

    result_is_ok = (test_pow_is_ok && result_is_ok);
  }

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
