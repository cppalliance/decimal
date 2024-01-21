// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Propagates up from boost.math
#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

#include <boost/core/lightweight_test.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/decimal.hpp>

#include <array>
#include <chrono>
#include <random>
#include <sstream>
#include <string>

namespace local
{
  template<typename IntegralTimePointType,
           typename ClockType = std::chrono::high_resolution_clock>
  auto time_point() noexcept -> IntegralTimePointType
  {
    using local_integral_time_point_type = IntegralTimePointType;
    using local_clock_type               = ClockType;

    const auto current_now =
      static_cast<std::uintmax_t>
      (
        std::chrono::duration_cast<std::chrono::nanoseconds>
        (
          local_clock_type::now().time_since_epoch()
        ).count()
      );

    return static_cast<local_integral_time_point_type>(current_now);
  }

  template<typename BigUintType>
  auto declexical_cast(const BigUintType& big_uint) -> std::string
  {
    std::stringstream strm;

    strm << std::dec << big_uint;

    return strm.str();
  }
}

template<typename BoostCtrlUint_Type,
         typename DecInternUint_Type>
auto test_big_uints() -> void
{
  using boost_ctrl_uint_type = BoostCtrlUint_Type;
  using dec_intern_uint_type = DecInternUint_Type;

  constexpr auto digits2 = std::numeric_limits<boost_ctrl_uint_type>::digits;

  using random_engine_type = std::mt19937_64;
  using bit_distribution_type = std::uniform_int_distribution<int>;

  random_engine_type    rng(local::time_point<typename random_engine_type::result_type>());
  bit_distribution_type bts(0, 1);

  bool lhs_is_fixed_and_near_max = true;
  bool rhs_is_fixed_and_variable = (!lhs_is_fixed_and_near_max);

  for(auto trials = static_cast<int>(INT8_C(0)); trials < static_cast<int>(INT16_C(0x200)); ++trials)
  {
    for(auto digits_split  = static_cast<int>(INT8_C(98));
             digits_split  > static_cast<int>(INT8_C(50));
             digits_split -= static_cast<int>(INT8_C( 5)))
    {
      const auto split = static_cast<float>(static_cast<float>(digits_split) / 100.0F);

      auto digits_lhs = static_cast<int>(digits2 - 1);
      auto digits_rhs = static_cast<int>(static_cast<float>((1.0F - split) * digits2));

      boost_ctrl_uint_type boost_ctrl_uint_lhs(1);
      dec_intern_uint_type dec_intern_uint_lhs(1);
      boost_ctrl_uint_type boost_ctrl_uint_rhs(1);
      dec_intern_uint_type dec_intern_uint_rhs(1);

      if(lhs_is_fixed_and_near_max)
      {
        boost_ctrl_uint_lhs <<= static_cast<unsigned>(digits2 - 1);
        dec_intern_uint_lhs <<= static_cast<unsigned>(digits2 - 1);
      }
      else
      {
        for(int i = 1; i < digits_lhs; ++i) { const int next_bit = bts(rng); boost_ctrl_uint_lhs <<= 1; dec_intern_uint_lhs <<= 1; if(next_bit != 0) { boost_ctrl_uint_lhs |= boost_ctrl_uint_type(1); dec_intern_uint_lhs |= dec_intern_uint_type(1); } }
      }

      if(rhs_is_fixed_and_variable)
      {
        boost_ctrl_uint_rhs <<= static_cast<unsigned>(digits_rhs);
        dec_intern_uint_rhs <<= static_cast<unsigned>(digits_rhs);
      }
      else
      {
        for(int i = 1; i < digits_rhs; ++i) { const int next_bit = bts(rng); boost_ctrl_uint_rhs <<= 1; dec_intern_uint_rhs <<= 1; if(next_bit != 0) { boost_ctrl_uint_rhs |= boost_ctrl_uint_type(1); dec_intern_uint_rhs |= dec_intern_uint_type(1); } }
      }

      lhs_is_fixed_and_near_max = (!lhs_is_fixed_and_near_max);
      rhs_is_fixed_and_variable = (!rhs_is_fixed_and_variable);

      const auto dec_intern_div = dec_intern_uint_lhs / dec_intern_uint_rhs;
      const auto boost_ctrl_div = boost_ctrl_uint_lhs / boost_ctrl_uint_rhs;

      BOOST_TEST(local::declexical_cast(dec_intern_div) == local::declexical_cast(boost_ctrl_div));
    }
  }
}

auto test_spot_uint256_t() -> void
{
  using boost_ctrl_uint_type = boost::multiprecision::uint256_t;
  using dec_intern_uint_type = boost::decimal::detail::uint256_t;

  {
    // Specially test several exactly-curated division operations that are know
    // to cover the hard-to-hit carry/borrow line(s) in Knuth long-division.

    using dec_intern_array_type = std::array<dec_intern_uint_type, static_cast<std::size_t>(UINT8_C(3))>;
    using boost_ctrl_array_type = std::array<boost_ctrl_uint_type, static_cast<std::size_t>(UINT8_C(3))>;

    dec_intern_array_type dec_intern_top_list =
    {{
      dec_intern_uint_type { { UINT64_C(0x01AC01E281D83F28), UINT64_C(0x698C19FD72AA8085) }, { UINT64_C(0x78F0CD3B0CD2FF5D), UINT64_C(0xD6A0A4DB3233D019) } },
      dec_intern_uint_type { { UINT64_C(0x009E1F4B3859275E), UINT64_C(0xE297AFBAB4ADB30B) }, { UINT64_C(0xFAE7A9D4CAF5672E), UINT64_C(0xB279A59B9906070C) } },
      dec_intern_uint_type { { UINT64_C(0x000B6F4866E326CC), UINT64_C(0x1321EAE5369D68E5) }, { UINT64_C(0x824E7315340514AB), UINT64_C(0x6EF6D107ECB8BC38) } }
    }};

    dec_intern_array_type dec_intern_bot_list =
    {{
      dec_intern_uint_type { { UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000) }, { UINT64_C(0x292FFA3C03F252D4), UINT64_C(0x42D1483A455B4281) } },
      dec_intern_uint_type { { UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000) }, { UINT64_C(0x2AA34E5021771CBC), UINT64_C(0x4EB1EFC17289FA09) } },
      dec_intern_uint_type { { UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000) }, { UINT64_C(0x718E8C8F6DBB6F76), UINT64_C(0x597BD68B19ACF237) } }
    }};

    boost_ctrl_array_type boost_ctrl_top_list =
    {{
      boost_ctrl_uint_type("0x1AC01E281D83F28698C19FD72AA808578F0CD3B0CD2FF5DD6A0A4DB3233D019"),
      boost_ctrl_uint_type("0x9E1F4B3859275EE297AFBAB4ADB30BFAE7A9D4CAF5672EB279A59B9906070C"),
      boost_ctrl_uint_type("0xB6F4866E326CC1321EAE5369D68E5824E7315340514AB6EF6D107ECB8BC38")
    }};

    boost_ctrl_array_type boost_ctrl_bot_list =
    {{
      boost_ctrl_uint_type("0x292FFA3C03F252D442D1483A455B4281"),
      boost_ctrl_uint_type("0x2AA34E5021771CBC4EB1EFC17289FA09"),
      boost_ctrl_uint_type("0x718E8C8F6DBB6F76597BD68B19ACF237")
    }};

    for(auto   index = static_cast<std::size_t>(UINT8_C(0));
               index < std::tuple_size<dec_intern_array_type>::value;
             ++index)
    {
      const auto dec_intern_div = dec_intern_top_list[index] / dec_intern_bot_list[index];
      const auto boost_ctrl_div = boost_ctrl_top_list[index] / boost_ctrl_bot_list[index];

      BOOST_TEST(local::declexical_cast(dec_intern_div) == local::declexical_cast(boost_ctrl_div));
    }
  }
}

int main()
{
  test_big_uints<boost::multiprecision::uint128_t, boost::decimal::detail::uint128>();
  test_big_uints<boost::multiprecision::uint256_t, boost::decimal::detail::uint256_t>();

  test_spot_uint256_t();

  return boost::report_errors();
}
