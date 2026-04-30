#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

class IHistoricalPriceFetcher
{
public:
    virtual ~IHistoricalPriceFetcher() = default;

    virtual std::vector<double> fetch_prices_for_ticker(
        const std::string &ticker,
        const std::string &announcement_date,
        int n) const = 0;

    virtual void fetch_all_group_prices(
        const std::vector<std::string> &beat_tickers,
        const std::vector<std::string> &meet_tickers,
        const std::vector<std::string> &miss_tickers,
        const std::map<std::string, std::string> &announcement_dates,
        int n,
        const std::function<void(const std::string &,
                                 std::vector<double>)> &callback) const = 0;

    virtual std::vector<double> fetch_benchmark_prices(
        int n,
        const std::string &earliest_announcement_date,
        const std::string &latest_announcement_date) const = 0;
};