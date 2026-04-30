// In AppCoordinator.cpp (teammate's file)
#include "ApiConfig.h"
#include "DataFetcher.h"

AppCoordinator::AppCoordinator()
{
    std::string token = ApiConfig::load_token(); // reads config.txt
    fetcher_ = std::make_unique<DataFetcher>(token);
}