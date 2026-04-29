#include "Stock.h"

#include <cmath>

Stock::Stock(std::string symbol, std::string sec,
             double surprise, double estimate,
             double reported, std::string date)
    : ticker(std::move(symbol)),
      sector(std::move(sec)),
      earning_surprise(surprise),
      eps_estimate(estimate),
      eps_reported(reported),
      announcement_date(std::move(date))
{
}

void Stock::calculate_daily_returns()
{
    daily_returns.clear();

    if (daily_prices.size() < 2)
    {
        return;
    }

    for (size_t i = 1; i < daily_prices.size(); ++i)
    {
        if (daily_prices[i - 1] > 0.0)
        {
            const double ret = std::log(daily_prices[i] / daily_prices[i - 1]);
            daily_returns.push_back(ret);
        }
        else
        {
            daily_returns.push_back(0.0);
        }
    }
}