#pragma once

#include <sstream>

template <typename T>
std::string toString(T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool isNumber(const std::string& s);
size_t conversionBytesParsing(const std::string& str);
