// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "where_file.hpp"
#include <boost/decimal.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace boost::decimal;

struct daily_data
{
    std::string date;
    decimal64_t open;
    decimal64_t high;
    decimal64_t low;
    decimal64_t close;
    decimal64_t volume;
};

// Function to split a CSV line into daily_data
auto parse_csv_line(const std::string& line) -> daily_data
{
    std::stringstream ss(line);
    std::string token;
    daily_data data;

    // Parse each column
    std::getline(ss, data.date, ',');
    std::getline(ss, token, ',');
    from_chars(token.c_str(), token.c_str() + token.size(), data.open);

    std::getline(ss, token, ',');
    from_chars(token.c_str(), token.c_str() + token.size(), data.high);

    std::getline(ss, token, ',');
    from_chars(token.c_str(), token.c_str() + token.size(), data.low);

    std::getline(ss, token, ',');
    from_chars(token.c_str(), token.c_str() + token.size(), data.close);

    std::getline(ss, token, ',');
    from_chars(token.c_str(), token.c_str() + token.size(), data.volume);

    return data;
}

int main()
{
    std::vector<daily_data> stock_data;
    const int window_size = 30;

    // Open and read the CSV file
    std::ifstream file(where_file("AAPL.csv"));
    std::string line;

    // Skip header line
    std::getline(file, line);

    // Read data
    while (std::getline(file, line))
    {
        stock_data.push_back(parse_csv_line(line));
    }

    // Calculate and print 30-day moving averages
    std::cout << "Date,30-Day Moving Average\n";

    size_t loop_count = 0; // Trivial counter to ensure this ran in the CI
    for (size_t i = window_size - 1; i < stock_data.size(); ++i)
    {
        decimal64_t sum(0);

        // Calculate sum for the window
        for (size_t j = 0; j < window_size; ++j)
        {
            sum += stock_data[i - j].close;
        }

        // Calculate average
        decimal64_t moving_avg = sum / decimal64_t(window_size);

        // Print result
        std::cout << stock_data[i].date << ","
                  << std::fixed << std::setprecision(2) << moving_avg << "\n";

        ++loop_count;
    }

    return loop_count == 0U;
}
