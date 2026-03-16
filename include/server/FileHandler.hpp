#pragma once

#include <string>

class FileHandler
{
public:
    static std::string getMimeType(const std::string& path);
    static std::string generateAutoIndex(const std::string& path, const std::string& uri);
};

