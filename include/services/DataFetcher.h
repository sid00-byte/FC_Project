#pragma once

// Rule 3 (self-contained): include own interface first, then STL.
// Rule 4 (include order): project header → STL headers.
#include "IHistoricalPriceFetcher.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

// Concrete fetcher implementing IHistoricalPriceFetcher via libcurl multi.
// All tickers are submitted as simultaneous async transfers in one event loop,
// so network wait time overlaps — total fetch time ≈ slowest single response
// rather than sum of all responses.
//
// Usage:
//   DataFetcher fetcher(ApiConfig::load_token());
//   fetcher.fetch_all_group_prices(beat, meet, miss, dates, n, callback);
class DataFetcher : public IHistoricalPriceFetcher
{
public:
    // api_token       : eodhistoricaldata.com token — load via ApiConfig
    // max_connections : simultaneous HTTP connections, capped at kMaxConnections
    //                   to stay within EOD rate limits
    explicit DataFetcher(const std::string &api_token,
                         long max_connections = 32);

    ~DataFetcher() override;

    // ── IHistoricalPriceFetcher overrides ─────────────────────────────────

    // Rule 6: const — reads api_token_ and max_connections_ only.
    std::vector<double> fetch_prices_for_ticker(
        const std::string &ticker,
        const std::string &announcement_date,
        int n) const override;

    // Rule 6: const — delegates to run_multi_fetch which is also const.
    void fetch_all_group_prices(
        const std::vector<std::string> &beat_tickers,
        const std::vector<std::string> &meet_tickers,
        const std::vector<std::string> &miss_tickers,
        const std::map<std::string, std::string> &announcement_dates,
        int n,
        const std::function<void(const std::string &,
                                 std::vector<double>)> &callback) const override;

    // Rule 6: marked const — does not modify object state.
    // N is validated against [kMinN, kMaxN]. AppCoordinator supplies the
    // earliest and latest announcement dates from the full stock universe
    // so the benchmark window covers every stock's event window.
    std::vector<double> fetch_benchmark_prices(
        int n,
        const std::string &earliest_announcement_date,
        const std::string &latest_announcement_date) const override;

private:
    // Build the EOD API URL for a given ticker and calendar date range.
    std::string build_url(const std::string &ticker,
                          const std::string &from_date,
                          const std::string &to_date) const;

    // Single blocking HTTP GET — used only for one-off fetches.
    // Throws std::runtime_error on curl failure or non-200 HTTP status,
    // including the ticker in the message for fast diagnosis.
    std::string http_get(const std::string &url) const;

    // Parse the CSV body returned by EOD into a chronological adjusted-close
    // price vector. Column layout: Date,Open,High,Low,Close,Adjusted_close,Volume
    // Throws std::runtime_error with the bad field value if parsing fails.
    static std::vector<double> parse_eod_csv(const std::string &csv_body);

    // Return the (from_date, to_date) calendar window for a ticker.
    // Adds kDateBufferDays on each side so weekends and public holidays never
    // cause us to come up short on trading days.
    static std::pair<std::string, std::string> date_window(
        const std::string &announcement_date, int n);

    // Validate that N is within the project-required range [30, 60].
    // Throws std::runtime_error with caller context if violated.
    static void validate_n(int n, const std::string &caller_context);

    // Core async engine: runs the curl_multi event loop over all tickers
    // and fires callback(ticker, prices) as each transfer completes.
    // Rule 6: const — only reads api_token_ and max_connections_.
    void run_multi_fetch(
        const std::vector<std::string> &tickers,
        const std::map<std::string, std::string> &announcement_dates,
        int n,
        const std::function<void(const std::string &,
                                 std::vector<double>)> &callback) const;

    // ── State ──────────────────────────────────────────────────────────────
    std::string api_token_;
    long max_connections_;

    static constexpr const char *kBaseUrl =
        "https://eodhistoricaldata.com/api/eod/";

    // Hard ceiling on simultaneous connections to respect EOD rate limits.
    static constexpr long kMaxConnections = 64L;

    // Calendar-day buffer added to each window to absorb weekends/holidays
    // and guarantee enough trading days are always returned.
    static constexpr int kDateBufferDays = 30;

    // Project requirement: N must be in [kMinN, kMaxN].
    static constexpr int kMinN = 30;
    static constexpr int kMaxN = 60;
};