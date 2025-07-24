// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>
#include <iomanip>

using namespace boost::decimal;

int main()
{
    const decimal_fast32_t fast_type {5};
    const std::uint32_t BID_bits {to_bid(fast_type)};
    const std::uint32_t DPD_bits {to_dpd(fast_type)};

    std::cout << std::hex
              << "BID format: " << BID_bits << '\n'
              << "DPD format: " << DPD_bits << std::endl;

    const decimal32_t bid_decimal {from_bid<decimal32_t>(BID_bits)};
    const decimal32_t dpd_decimal {from_dpd<decimal32_t>(DPD_bits)};

    return !(bid_decimal == dpd_decimal);
}
