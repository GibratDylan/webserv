#pragma once

#include <sstream>

template <typename T>
std::string toString(const T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool isNumber(const std::string& str);
size_t conversionBytesParsing(const std::string& str);
std::string trim(const std::string& str);
std::string addPath(const std::string& base, const std::string& suffix);
