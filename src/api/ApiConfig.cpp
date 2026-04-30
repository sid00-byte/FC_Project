#include "ApiConfig.h"

#include <fstream>
#include <stdexcept>
#include <string>

std::string ApiConfig::load_token(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error(
            "Cannot open API token file '" + path + "'. "
                                                    "Create the file and paste your EOD token on line 1.");
    }

    std::string token;
    std::getline(file, token);

    // Carriage returns from Windows line endings would silently corrupt the
    // token when appended to API URLs — strip them defensively.
    while (!token.empty() &&
           (token.back() == ' ' ||
            token.back() == '\r' ||
            token.back() == '\n'))
    {
        token.pop_back();
    }

    if (token.empty())
    {
        throw std::runtime_error(
            "API token file '" + path + "' is empty.");
    }

    return token;
}