// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt


#include <boost/decimal.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace boost::decimal;

auto where_file(const std::string& test_vectors_filename) -> std::string
{
    // Try to open the file in each of the known relative paths
    // in order to find out where it is located.

    // Boost-root
    std::string test_vectors_filename_relative = "libs/decimal/examples/" + test_vectors_filename;

    std::ifstream in_01(test_vectors_filename_relative.c_str());

    const bool file_01_is_open { in_01.is_open() };


    if(file_01_is_open)
    {
        in_01.close();
    }
    else
    {
        // Local test directory or IDE
        test_vectors_filename_relative = "../examples/" + test_vectors_filename;

        std::ifstream in_02(test_vectors_filename_relative.c_str());

        const bool file_02_is_open { in_02.is_open() };

        if(file_02_is_open)
        {
            in_02.close();
        }
        else
        {
            // test/cover
            test_vectors_filename_relative = "../../examples/" + test_vectors_filename;

            std::ifstream in_03(test_vectors_filename_relative.c_str());

            const bool file_03_is_open { in_03.is_open() };

            if(file_03_is_open)
            {
                in_03.close();
            }
            else
            {
                // CMake builds
                test_vectors_filename_relative = "../../../../libs/decimal/examples/" + test_vectors_filename;

                std::ifstream in_04(test_vectors_filename_relative.c_str());

                const bool file_04_is_open { in_04.is_open() };

                if(file_04_is_open)
                {
                    in_04.close();
                }
                else
                {
                    // Try to open the file from the absolute path.
                    test_vectors_filename_relative = test_vectors_filename;

                    std::ifstream in_05(test_vectors_filename_relative.c_str());

                    const bool file_05_is_open { in_05.is_open() };

                    if(file_05_is_open)
                    {
                        in_05.close();
                    }
                    else
                    {
                        // Clion Cmake builds
                        test_vectors_filename_relative = "../../../libs/decimal/examples/" + test_vectors_filename;

                        std::ifstream in_06(test_vectors_filename_relative.c_str());

                        const bool file_06_is_open { in_06.is_open() };
                        if (file_06_is_open)
                        {
                            in_06.close();
                        }
                        else
                        {
                            test_vectors_filename_relative = "";
                        }
                    }
                }
            }
        }
    }

    return test_vectors_filename_relative;
}

struct daily_data
{
    std::string date;
    decimal64 open;
    decimal64 high;
    decimal64 low;
    decimal64 close;
    decimal64 volume;
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
    while (std::getline(file, line)) {
        stock_data.push_back(parse_csv_line(line));
    }

    // Calculate and print 30-day moving averages
    std::cout << "Date,30-Day Moving Average\n";

    size_t loop_count = 0; // Trivial counter to ensure this ran in the CI
    for (size_t i = window_size - 1; i < stock_data.size(); ++i)
    {
        decimal64 sum(0);

        // Calculate sum for the window
        for (int j = 0; j < window_size; ++j) {
            sum += stock_data[i - j].close;
        }

        // Calculate average
        decimal64 moving_avg = sum / decimal64(window_size);

        // Print result
        std::cout << stock_data[i].date << ","
                  << std::fixed << std::setprecision(2) << moving_avg << "\n";

        ++loop_count;
    }

    return loop_count == 0U;
}
