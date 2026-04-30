#pragma once

#include <string>

// Loads the EOD API token from a plain-text config file so it is never
// hardcoded in source or committed to the repo.
//
// config.txt must contain the token on line 1 with no surrounding whitespace.
// Add config.txt to .gitignore before the first commit.
//
// Usage:
//   DataFetcher fetcher(ApiConfig::load_token());
class ApiConfig
{
public:
    // Reads the API token from the given file path.
    // Throws std::runtime_error if the file cannot be opened or is empty.
    static std::string load_token(const std::string &path = "config.txt");
};