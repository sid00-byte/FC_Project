#include "MarketManager.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <cstddef>
#include <iostream>
#include <vector>
#include <map>
#include <string>


#include "CSVParser.h"
#include "Stock.h"

// validator
void MarketManager::check_stock_map_not_empty() const
{
    if (stock_map.empty())
    {
        throw std::runtime_error("stock_map is empty");
    }
}

// clear data
void MarketManager::clear_data()
{
    beat_group.clear();
    meet_group.clear();
    miss_group.clear();
}

// organizing the stocks by their belonged sectors
std::map<std::string, std::vector<std::string>> MarketManager::build_sector_map() const
{
    std::map<std::string, std::vector<std::string>> sector_map;

    for (const auto& pair : stock_map)
    {
        const std::string& ticker = pair.first;
        const Stock& stock = pair.second;

        const std::string& raw_sector = stock.get_sector();
        const std::string sector = raw_sector.empty() ? "Unknown" : raw_sector;

        sector_map[sector].push_back(ticker);
    }

    return sector_map;
}

// sort the stocks in a sector by the suprise value in a descending order
void MarketManager::sort_by_surprise_desc(std::vector<std::string>& tickers) const
{
    // we use lambda expression to define a call-back function for sorting
    std::sort(tickers.begin(), tickers.end(),
              [this](const std::string& a, const std::string& b)
              {
                  return stock_map.at(a).get_earning_surprise() >
                         stock_map.at(b).get_earning_surprise();
              });
}

// remove the outliers
std::vector<std::string> MarketManager::remove_outliers(const std::vector<std::string>& tickers) const
{
    const size_t len_sec = tickers.size();
    const size_t len_outliers = static_cast<size_t>(std::floor(len_sec * 0.02));

    const size_t start = len_outliers;
    const size_t end = len_sec - len_outliers;

    if (start >= end)
    {
        std::cerr << "Warning: after removing the outliers, "
                     "there are no stocks left in the given sector.\n";
        return {};
    }

    return std::vector<std::string>(tickers.begin() + start, tickers.begin() + end);
}

// grouping the stocks after removing the outliers by the eps-surprise
void MarketManager::assign_to_groups(const std::vector<std::string>& retained)
{
    const size_t total = retained.size();
    const size_t group_size = total / 3;
    const size_t remainder = total % 3;

    const size_t beat_size = group_size + (remainder > 0 ? 1 : 0);
    const size_t meet_size = group_size + (remainder > 1 ? 1 : 0);
    const size_t miss_size = group_size;

    size_t idx = 0;

    for (size_t i = 0; i < beat_size; ++i)
    {
        beat_group.push_back(retained[idx]);
        stock_map.at(retained[idx]).set_group(kBeatGroup);
        ++idx;
    }

    for (size_t i = 0; i < meet_size; ++i)
    {
        meet_group.push_back(retained[idx]);
        stock_map.at(retained[idx]).set_group(kMeetGroup);
        ++idx;
    }

    for (size_t i = 0; i < miss_size; ++i)
    {
        miss_group.push_back(retained[idx]);
        stock_map.at(retained[idx]).set_group(kMissGroup);
        ++idx;
    }
}

// load stock data from the files to the stock_map
void MarketManager::load_stock_data(const std::string& components_file,
                                    const std::string& earnings_file)
{
    // use the CSV parser to parse the file for filling up the stock_map
    CSVParser parser;
    parser.load_earnings_csv(earnings_file, stock_map);
    parser.load_components_csv(components_file, stock_map);
}

// grouping the stocks
void MarketManager::group_sector()
{
    check_stock_map_not_empty();
    clear_data();

    auto sector_map = build_sector_map();

    for (auto& sec_pair : sector_map)
    {
        sort_by_surprise_desc(sec_pair.second);
        const std::vector<std::string> retained = remove_outliers(sec_pair.second);
        assign_to_groups(retained);
    }
}

bool MarketManager::contains(const std::string& ticker) const
{
    return stock_map.find(ticker) != stock_map.end();
}

const Stock& MarketManager::get_stock(const std::string& ticker) const
{
    const auto it = stock_map.find(ticker);

    if (it == stock_map.end())
    {
        throw std::runtime_error("Ticker not found: " + ticker);
    }

    return it->second;
}

Stock& MarketManager::get_stock(const std::string& ticker)
{
    auto it = stock_map.find(ticker);

    if (it == stock_map.end())
    {
        throw std::runtime_error("Ticker not found: " + ticker);
    }

    return it->second;
}

void MarketManager::set_daily_prices(const std::string& ticker, const std::vector<double>& prices)
{
    auto it = stock_map.find(ticker);

    if (it == stock_map.end())
    {
        throw std::runtime_error("Ticker not found: " + ticker);
    }

    it->second.set_daily_prices(prices);
}

void MarketManager::print_sector_report() const
{
    // TODO
}