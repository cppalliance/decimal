// Copyright 2024 Matt Borland
// Copyright 2024 Christoper Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/core/lightweight_test.hpp>

#include <random>

template<typename DecimalType, typename FloatType>
auto test_zeta() -> void
{
  using decimal_type = DecimalType;
  using float_type   = FloatType;

  // TODO(ckormanyos) Make actual tests whe the implementation is ready.
  BOOST_TEST_EQ(riemann_zeta(decimal_type { 15 , -1 }), decimal_type { 0 });

  {
    std::mt19937_64 gen;

    std::uniform_real_distribution<float> dist(static_cast<float_type>(1.1L), static_cast<float_type>(100.1L));

    for(auto i = static_cast<unsigned>(UINT8_C(0)); i < static_cast<unsigned>(UINT8_C(10)); ++i)
    {
      static_cast<void>(i);

      const decimal_type inf  { std::numeric_limits<decimal_type>::infinity() * static_cast<int>(dist(gen)) };
      const decimal_type nan  { std::numeric_limits<decimal_type>::quiet_NaN() * static_cast<int>(dist(gen)) };
      const decimal_type zero { decimal_type { 0 } * static_cast<int>(dist(gen)) };

      BOOST_TEST_EQ(riemann_zeta(inf), decimal_type { 1 } );
      BOOST_TEST   (isinf(riemann_zeta(-inf)) && signbit(riemann_zeta(-inf)));
      BOOST_TEST   (isnan(riemann_zeta(nan)));
      BOOST_TEST   (isnan(riemann_zeta(-nan)));

      const decimal_type minus_half { -5, -1 };

      BOOST_TEST_EQ(riemann_zeta(zero), minus_half);
    }
  }
}

int main()
{
    using decimal_type = ::boost::decimal::decimal32;

    test_zeta<decimal_type, float>();

    return boost::report_errors();
}
