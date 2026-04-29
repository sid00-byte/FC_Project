#pragma once

#include <string>
#include <vector>

class IStockUniverseWriter
{
    public:
        virtual bool contains(const std::string& ticker) const = 0;
        virtual void set_daily_prices(const std::string& ticker,
                                      const std::vector<double>& prices) = 0;

        virtual ~IStockUniverseWriter() = default;
};