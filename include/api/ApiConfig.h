#pragma once

#include <string>

class ApiConfig
{
public:
    static std::string load_token(const std::string &path = "api_token.txt");
};