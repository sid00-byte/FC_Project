#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

// Pure interface for fetching historical adjusted close prices from EOD.
// DataFetcher is the sole concrete implementation, using libcurl multi for
// async non-blocking I/O so all tickers are in-flight simultaneously.
// AppCoordinator and BootstrapEngine depend only on this interface,
// keeping them decoupled from the HTTP and parsing details.
class IHistoricalPriceFetcher
{
public:
    virtual ~IHistoricalPriceFetcher() = default;

    // Fetch 2N+1 adjusted close prices for a single ticker centred on its
    // earnings announcement date. Returns prices in chronological order.
    // N must satisfy 30 <= N <= 60 (project requirement).
    // Throws std::runtime_error on invalid N, network failure, or parse error.
    virtual std::vector<double> fetch_prices_for_ticker(
        const std::string &ticker,
        const std::string &announcement_date,
        int n) const = 0;

    // Fetch prices for every ticker across all three groups using async I/O.
    // Results are delivered via callback(ticker, prices) as each transfer
    // completes. Because the event loop is single-threaded, the callback fires
    // one at a time — no mutex is needed on the receiver side.
    // N must satisfy 30 <= N <= 60. Throws std::runtime_error on invalid N.
    virtual void fetch_all_group_prices(
        const std::vector<std::string> &beat_tickers,
        const std::vector<std::string> &meet_tickers,
        const std::vector<std::string> &miss_tickers,
        const std::map<std::string, std::string> &announcement_dates,
        int n,
        const std::function<void(const std::string &,
                                 std::vector<double>)> &callback) const = 0;

    // Fetch IWV (Russell 3000 ETF) adjusted close prices.
    // AppCoordinator computes earliest_date and latest_date from the full
    // set of announcement dates, then passes N so DataFetcher can add the
    // required buffer on each side. Matches sequence diagram step 9:
    // fetch_benchmark_prices(N).
    // N must satisfy 30 <= N <= 60. Throws std::runtime_error on invalid N.
    virtual std::vector<double> fetch_benchmark_prices(
        int n,
        const std::string &earliest_announcement_date,
        const std::string &latest_announcement_date) const = 0;
};