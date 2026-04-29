#pragma once

#include <map>
#include <string>
#include <vector>

#include "IGroupingService.h"
#include "IStockUniverseReader.h"
#include "IStockUniverseWriter.h"
#include "Stock.h"

class MarketManager: public IGroupingService,
                     public IStockUniverseReader,
                     public IStockUniverseWriter
{
    public:
        // prevent magic strings
        static constexpr const char* kBeatGroup = "Beat";
        static constexpr const char* kMeetGroup = "Meet";
        static constexpr const char* kMissGroup = "Miss";

    private:
        // storage of the stocks
        std::map<std::string, Stock> stock_map; // ticker -> Stock

        // grouping of the stocks
        std::vector<std::string> beat_group;
        std::vector<std::string> meet_group;
        std::vector<std::string> miss_group;

        // internal helpers
        void check_stock_map_not_empty() const;
        void clear_data();

        std::map<std::string, std::vector<std::string>> build_sector_map() const;
        void sort_by_surprise_desc(std::vector<std::string>& tickers) const;
        std::vector<std::string> remove_outliers(const std::vector<std::string>& tickers) const;
        void assign_to_groups(const std::vector<std::string>& retained);

    public:
        // data loader
        void load_stock_data(const std::string& components_file,
                             const std::string& earnings_file);

        // build sector-neutral groups
        void group_sector();

        // membership check
        bool contains(const std::string& ticker) const;

        // stock lookups
        const Stock& get_stock(const std::string& ticker) const;
        Stock& get_stock(const std::string& ticker);

        // stock map access
        const std::map<std::string, Stock>& get_all_stocks() const { return stock_map; }
        std::map<std::string, Stock>& get_all_stocks() { return stock_map; }

        // group accessors
        const std::vector<std::string>& get_beat_group() const { return beat_group; }
        const std::vector<std::string>& get_meet_group() const { return meet_group; }
        const std::vector<std::string>& get_miss_group() const { return miss_group; }
    
        // set daily prices
        void set_daily_prices(const std::string& ticker, const std::vector<double>& prices);
        
        // sector summary report (data preparation; UI formatting belongs elsewhere)
        void print_sector_report() const;
};