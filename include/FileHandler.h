#pragma once

#include <string>

class FileHandler
{
public:
    static bool fileExists(const std::string& path);
    static std::string readFile(const std::string& path);
    static std::string getMimeType(const std::string& path);
    static std::string generateAutoIndex(const std::string& path, const std::string& uri);
    static bool isDir(const std::string& path);
    static std::string normalizePath(const std::string& path);
    static bool deleteFile(const std::string& path);
    static bool writeFile(const std::string& path, const std::string& content);
};

