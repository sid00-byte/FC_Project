#include "ApiConfig.h"
#include "DataFetcher.h"

AppCoordinator::AppCoordinator()
{
    std::string token = ApiConfig::load_token();
    fetcher_ = std::make_unique<DataFetcher>(token);
}