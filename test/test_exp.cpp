// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

namespace local
{
  auto test_exp() -> bool
  {
    using decimal_type = boost::decimal::decimal32;

    auto result_is_ok = true;

    decimal_type arg { 0, 0 };

    const auto exp_result = exp(arg);

    const auto result_dummy_is_ok = (exp_result == 0);

    BOOST_TEST(result_dummy_is_ok);

    result_is_ok = (result_dummy_is_ok && result_is_ok);

    return result_is_ok;
  }
}

auto main() -> int
{
  auto result_is_ok = local::test_exp();

  result_is_ok = ((boost::report_errors() == 0) && result_is_ok);

  return (result_is_ok ? 0 : -1);
}
