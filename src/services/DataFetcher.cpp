#include "DataFetcher.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <curl/curl.h>

// libcurl invokes this for every chunk received on any connection.
// Routing through a per-handle std::string avoids temporary copies and lets
// the multi event loop accumulate responses independently per ticker.

static std::size_t curl_write_cb(char *ptr,
                                 std::size_t size,
                                 std::size_t nmemb,
                                 void *userdata)
{
    auto *buffer = static_cast<std::string *>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

// Binds a CURL* easy handle to its ticker name and response buffer.
// Heap-allocated via unique_ptr so the buffer address stays stable while
// libcurl holds a raw pointer to it through CURLOPT_WRITEDATA.

struct FetchContext
{
    std::string ticker;
    std::string buffer;
};

// Constructor / Destructor
DataFetcher::DataFetcher(const std::string &api_token, long max_connections)
    : api_token_(api_token), max_connections_(std::min(max_connections, kMaxConnections))
{
    // curl_global_init is not thread-safe and must be called exactly once
    // before any transfers begin — the constructor (single-threaded startup)
    // is the correct place for it.
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

DataFetcher::~DataFetcher()
{
    curl_global_cleanup();
}

void DataFetcher::validate_n(int n, const std::string &caller_context)
{
    if (n < kMinN || n > kMaxN)
    {
        throw std::runtime_error(
            caller_context + ": N=" + std::to_string(n) +
            " is out of the required range [" +
            std::to_string(kMinN) + ", " + std::to_string(kMaxN) + "].");
    }
}

std::string DataFetcher::build_url(const std::string &ticker,
                                   const std::string &from_date,
                                   const std::string &to_date) const
{
    std::ostringstream oss;
    oss << kBaseUrl << ticker << ".US"
        << "?from=" << from_date
        << "&to=" << to_date
        << "&period=d"
        << "&api_token=" << api_token_
        << "&fmt=csv";
    return oss.str();
}

// A plain N-day window would often fall short because weekends and holidays
// reduce trading days below 2N+1. kDateBufferDays of extra calendar days
// ensures we always retrieve enough data regardless of the announcement date.

std::pair<std::string, std::string> DataFetcher::date_window(
    const std::string &announcement_date, int n)
{
    std::tm tm_ann = {};
    std::istringstream ss(announcement_date);
    ss >> std::get_time(&tm_ann, "%Y-%m-%d");

    std::tm tm_start = tm_ann;
    tm_start.tm_mday -= (n + kDateBufferDays);
    std::mktime(&tm_start); // normalise across month/year boundaries

    std::tm tm_end = tm_ann;
    tm_end.tm_mday += (n + kDateBufferDays);
    std::mktime(&tm_end);

    auto fmt = [](const std::tm &t) -> std::string
    {
        char buf[11];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
        return std::string(buf);
    };

    return {fmt(tm_start), fmt(tm_end)};
}

// EOD CSV layout (header always present on line 1):
//   Date, Open, High, Low, Close, Adjusted_close, Volume
//   col:  0     1     2    3      4      5           6
// Only column 5 (Adjusted_close) is retained for return calculations.

std::vector<double> DataFetcher::parse_eod_csv(const std::string &csv_body)
{
    std::vector<double> prices;
    std::istringstream stream(csv_body);
    std::string line;

    bool header_skipped = false;

    while (std::getline(stream, line))
    {
        if (!header_skipped)
        {
            header_skipped = true;
            continue;
        }

        if (line.empty())
        {
            continue;
        }

        std::istringstream row(line);
        std::string field;
        int col = 0;

        while (std::getline(row, field, ','))
        {
            if (col == 5)
            {
                try
                {
                    prices.push_back(std::stod(field));
                }
                catch (const std::exception &e)
                {
                    throw std::runtime_error(
                        "Adjusted_close parse failure on value \"" + field +
                        "\": " + e.what());
                }
                break;
            }
            ++col;
        }
    }

    return prices;
}

std::string DataFetcher::http_get(const std::string &url) const
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("curl_easy_init() failed — cannot allocate handle.");
    }

    std::string body;
    long http_code = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        throw std::runtime_error(
            std::string("libcurl transfer error: ") + curl_easy_strerror(res) +
            " — URL: " + url);
    }

    if (http_code != 200)
    {
        throw std::runtime_error(
            "HTTP " + std::to_string(http_code) + " — URL: " + url);
    }

    return body;
}

std::vector<double> DataFetcher::fetch_prices_for_ticker(
    const std::string &ticker,
    const std::string &announcement_date,
    int n) const
{
    validate_n(n, "fetch_prices_for_ticker [ticker=" + ticker + "]");

    auto [from, to] = date_window(announcement_date, n);
    std::string url = build_url(ticker, from, to);
    std::string csv_body = http_get(url);

    std::vector<double> prices = parse_eod_csv(csv_body);

    // warn the user explicitly if there are not enough prices
    // for the requested 2N+1 event window around this announcement.
    const int required = 2 * n + 1;
    if (static_cast<int>(prices.size()) < required)
    {
        std::cerr << "[DataFetcher] Warning: ticker=" << ticker
                  << " has only " << prices.size()
                  << " trading days around announcement=" << announcement_date
                  << ", required=" << required
                  << " for N=" << n << ".\n";
    }

    return prices;
}

// The IWV benchmark must cover every stock's [-N, +N] event window.
// AppCoordinator supplies the earliest and latest announcement dates across
// the full universe; we expand each boundary by N + kDateBufferDays so the
// IWV series spans all stocks regardless of weekends or public holidays.

std::vector<double> DataFetcher::fetch_benchmark_prices(
    int n,
    const std::string &earliest_announcement_date,
    const std::string &latest_announcement_date) const
{
    validate_n(n, "fetch_benchmark_prices");

    // from = earliest date pushed back by N + buffer
    // to   = latest date pushed forward by N + buffer
    auto [from, ignored_end] = date_window(earliest_announcement_date, n);
    auto [ignored_start, to] = date_window(latest_announcement_date, n);

    std::string url = build_url("IWV", from, to);
    std::string csv_body = http_get(url);
    return parse_eod_csv(csv_body);
}

// curl_multi drives all connections concurrently on one thread.  While one
// response is in flight, the socket for every other ticker is also open and
// advancing — total wall time ≈ slowest individual response, not the sum.
//
// Memory safety: each CURL* easy handle is paired with a std::unique_ptr<FetchContext>
// stored in handle_map.  Ownership transfers to the cleanup block when a
// transfer finishes, so no raw delete is needed and leaks are impossible even
// on early-exit paths.

void DataFetcher::run_multi_fetch(
    const std::vector<std::string> &tickers,
    const std::map<std::string, std::string> &announcement_dates,
    int n,
    const std::function<void(const std::string &,
                             std::vector<double>)> &callback) const
{
    if (tickers.empty())
    {
        return;
    }

    //1. Create multi handle and cap concurrent connections
    CURLM *multi = curl_multi_init();
    if (!multi)
    {
        throw std::runtime_error("curl_multi_init() failed.");
    }

    curl_multi_setopt(multi, CURLMOPT_MAX_TOTAL_CONNECTIONS, max_connections_);
    curl_multi_setopt(multi, CURLMOPT_MAX_HOST_CONNECTIONS, max_connections_);

    //2. Register one easy handle per ticker
    // unique_ptr ownership: released into handle_map; reclaimed in cleanup.
    std::map<CURL *, std::unique_ptr<FetchContext>> handle_map;

    for (const auto &ticker : tickers)
    {
        auto it = announcement_dates.find(ticker);
        if (it == announcement_dates.end())
        {
            std::cerr << "[DataFetcher] No announcement date for ticker="
                      << ticker << " — skipping.\n";
            continue;
        }

        auto [from, to] = date_window(it->second, n);
        std::string url = build_url(ticker, from, to);

        CURL *easy = curl_easy_init();
        if (!easy)
        {
            std::cerr << "[DataFetcher] curl_easy_init() failed for ticker="
                      << ticker << " — skipping.\n";
            continue;
        }

        // Construct context inside the map to guarantee stable buffer address.
        auto ctx = std::make_unique<FetchContext>();
        ctx->ticker = ticker;

        curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
        curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, curl_write_cb);
        curl_easy_setopt(easy, CURLOPT_WRITEDATA, &ctx->buffer);
        curl_easy_setopt(easy, CURLOPT_TIMEOUT, 30L);

        curl_multi_add_handle(multi, easy);
        handle_map[easy] = std::move(ctx);
    }

    //3 & 4. Event loop
    int still_running = 1;
    long completed = 0;
    long total = static_cast<long>(handle_map.size());

    while (still_running)
    {
        CURLMcode mc = curl_multi_perform(multi, &still_running);
        if (mc != CURLM_OK)
        {
            std::cerr << "[DataFetcher] curl_multi_perform: "
                      << curl_multi_strerror(mc) << "\n";
            break;
        }

        //5. Drain completed transfers
        int msgs_left = 0;
        CURLMsg *msg = nullptr;

        while ((msg = curl_multi_info_read(multi, &msgs_left)))
        {
            if (msg->msg != CURLMSG_DONE)
            {
                continue;
            }

            CURL *easy = msg->easy_handle;
            FetchContext &ctx = *handle_map[easy];

            if (msg->data.result == CURLE_OK)
            {
                long http_code = 0;
                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);

                if (http_code == 200)
                {
                    try
                    {
                        std::vector<double> prices = parse_eod_csv(ctx.buffer);

                        // warn user if trading-day count is
                        // insufficient for the requested event window.
                        const int required = 2 * n + 1;
                        if (static_cast<int>(prices.size()) < required)
                        {
                            std::cerr << "[DataFetcher] Warning: ticker="
                                      << ctx.ticker
                                      << " has only " << prices.size()
                                      << " trading days, required=" << required
                                      << " for N=" << n << ".\n";
                        }

                        // Single-threaded delivery — callback fires one at a
                        // time, so no mutex is needed on the receiver side.
                        callback(ctx.ticker, std::move(prices));
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "[DataFetcher] CSV parse error ticker="
                                  << ctx.ticker << ": " << e.what() << "\n";
                    }
                }
                else
                {
                    std::cerr << "[DataFetcher] HTTP " << http_code
                              << " ticker=" << ctx.ticker << "\n";
                }
            }
            else
            {
                std::cerr << "[DataFetcher] Transfer failed ticker="
                          << ctx.ticker << ": "
                          << curl_easy_strerror(msg->data.result) << "\n";
            }

            ++completed;
            if (completed % 10 == 0 || completed == total)
            {
                std::cout << "[DataFetcher] " << completed
                          << "/" << total << " tickers fetched.\n";
            }

            //6. Release this handle — unique_ptr cleans up FetchContext
            curl_multi_remove_handle(multi, easy);
            curl_easy_cleanup(easy);
            handle_map.erase(easy);
        }

        // Yield until any socket is readable — prevents a busy-poll CPU spin
        // while all in-flight transfers are waiting on network I/O.
        if (still_running)
        {
            curl_multi_wait(multi, nullptr, 0, 500, nullptr);
        }
    }

    //release any handles that did not complete
    for (auto &[easy, ctx] : handle_map)
    {
        curl_multi_remove_handle(multi, easy);
        curl_easy_cleanup(easy);
    }
    // unique_ptrs in handle_map free FetchContext objects automatically here.

    curl_multi_cleanup(multi);
}

void DataFetcher::fetch_all_group_prices(
    const std::vector<std::string> &beat_tickers,
    const std::vector<std::string> &meet_tickers,
    const std::vector<std::string> &miss_tickers,
    const std::map<std::string, std::string> &announcement_dates,
    int n,
    const std::function<void(const std::string &,
                             std::vector<double>)> &callback) const
{
    validate_n(n, "fetch_all_group_prices");

    std::vector<std::string> all_tickers;
    all_tickers.reserve(
        beat_tickers.size() + meet_tickers.size() + miss_tickers.size());

    all_tickers.insert(all_tickers.end(),
                       beat_tickers.begin(), beat_tickers.end());
    all_tickers.insert(all_tickers.end(),
                       meet_tickers.begin(), meet_tickers.end());
    all_tickers.insert(all_tickers.end(),
                       miss_tickers.begin(), miss_tickers.end());

    std::cout << "[DataFetcher] Starting async fetch for "
              << all_tickers.size() << " tickers ("
              << max_connections_ << " concurrent connections, N="
              << n << ")...\n";

    run_multi_fetch(all_tickers, announcement_dates, n, callback);

    std::cout << "[DataFetcher] All fetches complete.\n";
}