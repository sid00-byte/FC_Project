// Standalone integration test for DataFetcher

#include "ApiConfig.h"
#include "DataFetcher.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

static void print_separator(const std::string &title)
{
    std::cout << "\n"
              << std::string(60, '=') << "\n"
              << "  " << title << "\n"
              << std::string(60, '=') << "\n";
}

static void print_prices(const std::string &ticker,
                         const std::vector<double> &prices,
                         int preview = 5)
{
    std::cout << ticker << ": " << prices.size() << " prices  |  first "
              << preview << ": ";

    for (int i = 0; i < preview && i < static_cast<int>(prices.size()); ++i)
    {
        std::cout << prices[i] << " ";
    }
    std::cout << "\n";
}

int main()
{
    std::string api_token;
    try
    {
        api_token = ApiConfig::load_token();
        std::cout << "API token loaded successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Startup error: " << e.what() << "\n";
        return 1;
    }

    DataFetcher fetcher(api_token, 32);

    // Test 1: fetch_prices_for_ticker
    print_separator("Test 1: fetch_prices_for_ticker (single, blocking)");
    try
    {
        auto prices = fetcher.fetch_prices_for_ticker("AAPL", "2025-10-30", 30);
        print_prices("AAPL", prices);

        const int required = 2 * 30 + 1;
        if (static_cast<int>(prices.size()) < required)
        {
            std::cout << "WARNING: fewer than " << required
                      << " trading days returned.\n";
        }
        else
        {
            std::cout << "PASS\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << "\n";
    }

    // Test 2: validate_n rejects out-of-range N
    print_separator("Test 2: N validation (expect exception for N=20)");
    try
    {
        fetcher.fetch_prices_for_ticker("AAPL", "2025-10-30", 20);
        std::cout << "FAIL: expected exception not thrown.\n";
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "PASS: correctly rejected N=20 — " << e.what() << "\n";
    }

    // Test 3: fetch_benchmark_prices
    print_separator("Test 3: fetch_benchmark_prices (IWV, N=30)");
    try
    {
        // Pass N plus the earliest and latest announcement dates from the
        // Q3 2025 universe — AppCoordinator provides these in real usage.
        auto iwv = fetcher.fetch_benchmark_prices(
            30,
            "2025-10-23",  // earliest in this test set (TSLA)
            "2025-11-20"); // latest in this test set  (NVDA)
        print_prices("IWV", iwv);
        std::cout << (iwv.empty() ? "FAIL: no prices returned." : "PASS") << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << "\n";
    }

    // Test 4: fetch_all_group_prices (curl_multi async)
    print_separator("Test 4: fetch_all_group_prices (curl_multi, N=30)");

    // Replace with real tickers and dates from your earnings Excel sheet
    // once CSVParser teammate's output is available.
    std::vector<std::string> beat = {"AAPL", "MSFT", "NVDA"};
    std::vector<std::string> meet = {"GOOGL", "AMZN"};
    std::vector<std::string> miss = {"META", "TSLA"};

    std::map<std::string, std::string> dates =
        {
            {"AAPL", "2025-10-30"},
            {"MSFT", "2025-10-29"},
            {"NVDA", "2025-11-20"},
            {"GOOGL", "2025-10-28"},
            {"AMZN", "2025-10-31"},
            {"META", "2025-10-29"},
            {"TSLA", "2025-10-23"},
        };

    std::map<std::string, std::vector<double>> results;

    try
    {
        fetcher.fetch_all_group_prices(
            beat, meet, miss, dates, 30,
            [&](const std::string &ticker, std::vector<double> prices)
            {
                // No mutex needed — curl_multi fires callback single-threadedly.
                results[ticker] = std::move(prices);
            });

        std::cout << "\nResults (" << results.size() << " tickers):\n";
        for (const auto &[ticker, prices] : results)
        {
            print_prices(ticker, prices);
        }

        int missing = 0;
        for (const auto &[ticker, date] : dates)
        {
            if (results.find(ticker) == results.end())
            {
                std::cerr << "MISSING: " << ticker << "\n";
                ++missing;
            }
        }
        std::cout << (missing == 0 ? "PASS: all tickers fetched.\n"
                                   : "FAIL: some tickers missing.\n");
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << "\n";
    }

    return 0;
}