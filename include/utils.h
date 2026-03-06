#pragma once

#include <sstream>
#include <vector>

template <typename T>
std::string toString(T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool isNumber(const std::string& str);
size_t conversionBytesParsing(const std::string& str);
bool isExtensionMatch(const std::string& path, const std::string& extensions);
