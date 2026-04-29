#pragma once

#include <string>
#include <vector>

class Stock
{
    private:
        // identity information of the stock
        std::string ticker;
        std::string sector;
        std::string group;

        // financial report info of the stock
        double earning_surprise;
        double eps_estimate;
        double eps_reported;
        std::string announcement_date;

        // price series and return series
        std::vector<double> daily_prices;
        std::vector<double> daily_returns;

    public:
        Stock(std::string symbol, std::string sec,
              double surprise, double estimate,
              double reported, std::string date);

        // getters
        const std::string& get_ticker() const { return ticker; }
        const std::string& get_sector() const { return sector; }
        const std::string& get_group() const { return group; }
        const std::string& get_announcement_date() const { return announcement_date; }

        double get_earning_surprise() const { return earning_surprise; }
        double get_eps_estimate() const { return eps_estimate; }
        double get_eps_reported() const { return eps_reported; }

        const std::vector<double>& get_daily_prices() const { return daily_prices; }
        const std::vector<double>& get_daily_returns() const { return daily_returns; }

        // setters
        void set_ticker(const std::string& t) { ticker = t; }
        void set_sector(const std::string& sec) { sector = sec; }
        void set_group(const std::string& g) { group = g; }
        void set_daily_prices(const std::vector<double>& prices) { daily_prices = prices; }

        // domain operation
        void calculate_daily_returns();
};