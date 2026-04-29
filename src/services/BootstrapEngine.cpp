#include "BootstrapEngine.h"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "MarketManager.h"
#include "Matrix.h"
#include "Stock.h"

BootstrapEngine::BootstrapEngine(const MarketManager& mng,
                                 const std::map<std::string, std::vector<double>>& brp)
    : manager(mng),
      benchmark_return_map(brp),
      n(0),
      repetitions(40),
      sample_size(30),
      expected_aar(),
      aar_std(),
      expected_caar(),
      caar_std()
{
}

void BootstrapEngine::validate_inputs() const
{
    // validate the range of n
    if (n < 30 || n > 60)
    {
        throw std::invalid_argument(
            "The window size N must be a number between 30 and 60, current N = " 
            + std::to_string(n)
        );
    }

    // validate other hyper-parameters
    if (repetitions <= 0 || sample_size <= 0)
    {
        throw std::invalid_argument(
            "The hyperparameters should be larger than 0, current repetitions = "
            + std::to_string(repetitions) 
            + ", current sample_size = " + std::to_string(sample_size)
        );
    }

    // check if the group size is enough for samping
    const std::vector<std::string>& beat = manager.get_beat_group();
    const std::vector<std::string>& meet = manager.get_meet_group();
    const std::vector<std::string>& miss = manager.get_miss_group();
    if (beat.size() < sample_size || meet.size() < sample_size || miss.size() < sample_size)
    {
        throw std::runtime_error(
            "At least one group has fewer than sample_size = " +
            std::to_string(sample_size) + " stocks");
    }

    // every stock should have 2N daily returns with the corresponding benchmark series
    const size_t expected_len = 2 * n;
    auto check_group = [&](const std::vector<std::string>& group, const std::string& group_name)
    {
        for (const std::string& ticker: group)
        {
            // check a stock from the current group
            const Stock& stock = manager.get_stock(ticker);

            // check if the length of the stock daily return series is valid
            if (stock.get_daily_returns().size() != expected_len)
            {
                throw std::runtime_error(
                    std::string("Stock ") + ticker + " in group " + group_name +
                    " has " + std::to_string(stock.get_daily_returns().size()) +
                    " returns; expected " + std::to_string(expected_len)
                );
            }

            // check if the announcement date is in the current window
            const auto it = benchmark_return_map.find(stock.get_announcement_date());
            if (it == benchmark_return_map.end())
            {
                throw std::runtime_error(
                    std::string("Benchmark returns missing for ") + ticker +
                    " on " + stock.get_announcement_date()
                  );
            }

            // check if the benchmark has the return rate series of length 2N
            if (it->second.size() != expected_len)
            {
                throw std::runtime_error(
                    std::string("Benchmark returns for ") + ticker +
                    " on " + stock.get_announcement_date() + " have " +
                    std::to_string(it->second.size()) + " entries; expected " +
                    std::to_string(expected_len)
                  );
            }
        }
    };

    check_group(manager.get_beat_group(), MarketManager::kBeatGroup);
    check_group(manager.get_meet_group(), MarketManager::kMeetGroup);
    check_group(manager.get_miss_group(), MarketManager::kMissGroup);

}

size_t BootstrapEngine::group_name_to_row(const std::string& group_name) const
{
    // mapping the group_name to the row indices in the market matrix
    if (group_name == MarketManager::kBeatGroup) return 0;
    if (group_name == MarketManager::kMeetGroup) return 1;
    if (group_name == MarketManager::kMissGroup) return 2;

    throw std::invalid_argument("Unknown group name: " + group_name);
}

std::vector<double> BootstrapEngine::compute_abnormal_returns(
    const std::vector<double>& stock_returns,
    const std::vector<double>& benchmark_returns) const
{
    // validation
    if (stock_returns.size() != benchmark_returns.size())
    {
        throw std::runtime_error("The dimension of the two return paths mismatch");
    }

    // get the abnormal returns
    std::vector<double> abnormal_returns(stock_returns.size(), 0.0);
    for (size_t i = 0; i < stock_returns.size(); ++i)
    {
        abnormal_returns[i] = stock_returns[i] - benchmark_returns[i];
    }
    return abnormal_returns;
}

void BootstrapEngine::prepare_abnormal_returns()
{
    // clear the current container
    abnormal_return_map.clear();

    // define a lambda function to process each group
    auto process_group = [this](const std::vector<std::string>& group)
    {
        for (const auto& ticker : group)
        {
            const Stock& stock = manager.get_stock(ticker);
            const std::vector<double>& stock_returns = stock.get_daily_returns();

            const auto it = benchmark_return_map.find(stock.get_announcement_date());
            if (it == benchmark_return_map.end())
            {
                throw std::runtime_error(
                    "Benchmark returns missing for announcement date: " +
                    stock.get_announcement_date());
            }
            const std::vector<double>& benchmark_returns = it->second;

            abnormal_return_map[ticker] =
                compute_abnormal_returns(stock_returns, benchmark_returns);
        }
    };

    process_group(manager.get_beat_group());
    process_group(manager.get_meet_group());
    process_group(manager.get_miss_group());
}

std::vector<std::string> BootstrapEngine::sample_from_a_group(const std::vector<std::string>& group) const
{
    if (group.size() < sample_size)
    {
        throw std::runtime_error("There's not enough data for sampling");
    }

    std::vector<std::string> sample = group;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(sample.begin(), sample.end(), gen);

    sample.resize(sample_size);
    return sample;
}

std::vector<double> BootstrapEngine::compute_group_aar(const std::vector<std::string>& sample) const
{
    if (sample.empty())
    {
        throw std::runtime_error("Sample is empty");
    }

    std::vector<double> aar(2 * n, 0.0);

    for (const std::string& ticker : sample)
    {
        const auto it = abnormal_return_map.find(ticker);
        if (it == abnormal_return_map.end())
        {
            throw std::runtime_error("Abnormal returns missing for ticker: " + ticker);
        }

        const std::vector<double>& ar = it->second;

        for (size_t i = 0; i < ar.size(); ++i)
        {
            aar[i] += ar[i];
        }
    }

    for (double& value : aar)
    {
        value /= sample.size();
    }

    return aar;
}

std::vector<double> BootstrapEngine::compute_caar_from_aar(const std::vector<double>& aar) const
{
    if (aar.empty())
    {
        throw std::invalid_argument("Input AAR vector is empty");
    }

    std::vector<double> caar(aar.size(), 0.0);

    caar[0] = aar[0];
    for (size_t i = 1; i < aar.size(); ++i)
    {
        caar[i] = caar[i - 1] + aar[i];
    }

    return caar;
}

void BootstrapEngine::run_group_bootstrap(const std::string& group_name)
{
    const std::vector<std::string>* group_ptr = nullptr;

    if (group_name == MarketManager::kBeatGroup)
    {
        group_ptr = &manager.get_beat_group();
    }
    else if (group_name == MarketManager::kMeetGroup)
    {
        group_ptr = &manager.get_meet_group();
    }
    else if (group_name == MarketManager::kMissGroup)
    {
        group_ptr = &manager.get_miss_group();
    }
    else
    {
        throw std::invalid_argument("Unknown group name: " + group_name);
    }

    const std::vector<std::string>& group = *group_ptr;
    const size_t row = group_name_to_row(group_name);

    Matrix aar_samples;
    Matrix caar_samples;

    for (size_t rep = 0; rep < repetitions; ++rep)
    {
        const std::vector<std::string> sample = sample_from_a_group(group);
        const std::vector<double> aar = compute_group_aar(sample);
        const std::vector<double> caar = compute_caar_from_aar(aar);

        aar_samples.push_back_row(aar);
        caar_samples.push_back_row(caar);
    }

    const std::vector<double> mean_aar = aar_samples.calculate_mean();
    const std::vector<double> std_aar = aar_samples.calculate_std();
    const std::vector<double> mean_caar = caar_samples.calculate_mean();
    const std::vector<double> std_caar = caar_samples.calculate_std();

    for (size_t col = 0; col < 2 * n; ++col)
    {
        expected_aar.set_value(row, col, mean_aar[col]);
        aar_std.set_value(row, col, std_aar[col]);
        expected_caar.set_value(row, col, mean_caar[col]);
        caar_std.set_value(row, col, std_caar[col]);
    }
}

void BootstrapEngine::run(size_t event_window_n,
                          size_t repetitions_in,
                          size_t sample_size_in)
{
    n = event_window_n;
    repetitions = repetitions_in;
    sample_size = sample_size_in;

    abnormal_return_map.clear();

    expected_aar = Matrix(3, 2 * n);
    aar_std = Matrix(3, 2 * n);
    expected_caar = Matrix(3, 2 * n);
    caar_std = Matrix(3, 2 * n);

    validate_inputs();
    prepare_abnormal_returns();

    run_group_bootstrap(MarketManager::kBeatGroup);
    run_group_bootstrap(MarketManager::kMeetGroup);
    run_group_bootstrap(MarketManager::kMissGroup);
}