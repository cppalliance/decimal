// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>
#include <iomanip>

using namespace boost::decimal;

auto convert_currency(decimal64_t amount, decimal64_t exchange_rate) -> decimal64_t
{
    return amount * exchange_rate;
}

int main()
{
    const auto usd_amount = strtod64("1000.50", nullptr);
    const auto usd_to_eur_rate = strtod64("0.92", nullptr);

    const decimal64_t eur_amount = convert_currency(usd_amount, usd_to_eur_rate);
    constexpr decimal64_t exact_eur_amount(92046, -2);

    std::cout << "USD: " << std::fixed << std::setprecision(2) << usd_amount << "\n";
    std::cout << "EUR: " << std::fixed << std::setprecision(2) << eur_amount << "\n";

    return !(eur_amount == exact_eur_amount);
}
