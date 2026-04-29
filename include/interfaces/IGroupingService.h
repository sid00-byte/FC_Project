#pragma once

#include <string>

class IGroupingService
{
    public:
        virtual void load_stock_data(const std::string& components_file,
                                     const std::string& earnings_file) = 0;
        virtual void group_sector() = 0;
        virtual void print_sector_report() const = 0;

        virtual ~IGroupingService() = default;
};