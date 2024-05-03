// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <array>
#include <limits>
#include <cstdint>

template <typename T>
void test()
{
    constexpr std::array<std::uint64_t, 20> powers_of_10 =
    {{
             UINT64_C(1), UINT64_C(10), UINT64_C(100), UINT64_C(1000), UINT64_C(10000), UINT64_C(100000), UINT64_C(1000000),
             UINT64_C(10000000), UINT64_C(100000000), UINT64_C(1000000000), UINT64_C(10000000000), UINT64_C(100000000000),
             UINT64_C(1000000000000), UINT64_C(10000000000000), UINT64_C(100000000000000), UINT64_C(1000000000000000),
             UINT64_C(10000000000000000), UINT64_C(100000000000000000), UINT64_C(1000000000000000000), UINT64_C(10000000000000000000)
     }};

    for (const auto& val : powers_of_10)
    {
        if (val < std::numeric_limits<T>::max())
        {
            const auto temp {boost::decimal::detail::remove_trailing_zeros(static_cast<T>(val))};
            if (!BOOST_TEST_EQ(temp.trimmed_number, T(1)))
            {
                std::cerr << "Input Number: " << val
                          << "\nOutput Number: " << temp.trimmed_number
                          << "\nZeros removed: " << temp.number_of_removed_zeros << std::endl;
            }
        }
    }
}

void test_extended()
{
    using namespace boost::decimal;
    constexpr std::array<detail::uint128, 18> powers_of_10 =
    {{
        detail::uint128 {UINT64_C(0x5), UINT64_C(0x6BC75E2D63100000)},
        detail::uint128 {UINT64_C(0x36), UINT64_C(0x35C9ADC5DEA00000)},
        detail::uint128 {UINT64_C(0x21E), UINT64_C(0x19E0C9BAB2400000)},
        detail::uint128 {UINT64_C(0x152D), UINT64_C(0x02C7E14AF6800000)},
        detail::uint128 {UINT64_C(0x84595), UINT64_C(0x161401484A000000)},
        detail::uint128 {UINT64_C(0x52B7D2), UINT64_C(0xDCC80CD2E4000000)},
        detail::uint128 {UINT64_C(0x33B2E3C), UINT64_C(0x9FD0803CE8000000)},
        detail::uint128 {UINT64_C(0x204FCE5E), UINT64_C(0x3E25026110000000)},
        detail::uint128 {UINT64_C(0x1431E0FAE), UINT64_C(0x6D7217CAA0000000)},
        detail::uint128 {UINT64_C(0xC9F2C9CD0), UINT64_C(0x4674EDEA40000000)},
        detail::uint128 {UINT64_C(0x7E37BE2022), UINT64_C(0xC0914B2680000000)},
        detail::uint128 {UINT64_C(0x4EE2D6D415B), UINT64_C(0x85ACEF8100000000)},
        detail::uint128 {UINT64_C(0x314DC6448D93), UINT64_C(0x38C15B0A00000000)},
        detail::uint128 {UINT64_C(0x1ED09BEAD87C0), UINT64_C(0x378D8E6400000000)},
        detail::uint128 {UINT64_C(0x13426172C74D82), UINT64_C(0x2B878FE800000000)},
        detail::uint128 {UINT64_C(0xC097CE7BC90715), UINT64_C(0xB34B9F1000000000)},
        detail::uint128 {UINT64_C(0x785EE10D5DA46D9), UINT64_C(0x00F436A000000000)},
        detail::uint128 {UINT64_C(0x4B3B4CA85A86C47A), UINT64_C(0x098A224000000000)}
     }};

    for (const auto& val : powers_of_10)
    {
        const auto temp {boost::decimal::detail::remove_trailing_zeros(val)};
        if (!BOOST_TEST_EQ(temp.trimmed_number, detail::uint128(1)))
        {
            std::cerr << "Input Number: " << val
                      << "\nOutput Number: " << temp.trimmed_number
                      << "\nZeros removed: " << temp.number_of_removed_zeros << std::endl;
        }
    }
}

int main()
{
    test<std::uint32_t>();
    test<std::uint64_t>();
    test<boost::decimal::detail::uint128>();

    test_extended();

    return boost::report_errors();
}
