#pragma once

#include <string>
#include <vector>

class Stock;

class IStockUniverseReader
{
    public:
        // group getters
        virtual const std::vector<std::string>& get_beat_group() const = 0;
        virtual const std::vector<std::string>& get_meet_group() const = 0;
        virtual const std::vector<std::string>& get_miss_group() const = 0;

        // stock getter 
        virtual const Stock& get_stock(const std::string& ticker) const = 0;

        // virtual destructor
        virtual ~IStockUniverseReader() = default; // use the defualt destructor later
};
