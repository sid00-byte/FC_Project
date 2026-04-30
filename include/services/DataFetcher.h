#pragma once

#include "IHistoricalPriceFetcher.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

class DataFetcher : public IHistoricalPriceFetcher
{
public:

    explicit DataFetcher(const std::string &api_token,
                         long max_connections = 32);

    ~DataFetcher() override;

    std::vector<double> fetch_prices_for_ticker(
        const std::string &ticker,
        const std::string &announcement_date,
        int n) const override;

    void fetch_all_group_prices(
        const std::vector<std::string> &beat_tickers,
        const std::vector<std::string> &meet_tickers,
        const std::vector<std::string> &miss_tickers,
        const std::map<std::string, std::string> &announcement_dates,
        int n,
        const std::function<void(const std::string &,
                                 std::vector<double>)> &callback) const override;

    std::vector<double> fetch_benchmark_prices(
        int n,
        const std::string &earliest_announcement_date,
        const std::string &latest_announcement_date) const override;

private:
    std::string build_url(const std::string &ticker,
                          const std::string &from_date,
                          const std::string &to_date) const;

    std::string http_get(const std::string &url) const;

    static std::vector<double> parse_eod_csv(const std::string &csv_body);

    static std::pair<std::string, std::string> date_window(
        const std::string &announcement_date, int n);

    static void validate_n(int n, const std::string &caller_context);

    void run_multi_fetch(
        const std::vector<std::string> &tickers,
        const std::map<std::string, std::string> &announcement_dates,
        int n,
        const std::function<void(const std::string &,
                                 std::vector<double>)> &callback) const;

    std::string api_token_;
    long max_connections_;

    static constexpr const char *kBaseUrl =
        "https://eodhistoricaldata.com/api/eod/";

    static constexpr long kMaxConnections = 64L;

    static constexpr int kDateBufferDays = 30;

    static constexpr int kMinN = 30;
    static constexpr int kMaxN = 60;
};