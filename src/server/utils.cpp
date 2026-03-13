#include "../../include/server/utils.hpp"

#include <cstdlib>
#include <limits>
#include <map>
#include <string>

bool isNumber(const std::string& str) {
	if (str.empty()) {
		return false;
	}

	for (size_t i = 0; i < str.length(); ++i) {
		if (!std::isdigit(str[i])) {
			return false;
		}
	}
	return true;
}

size_t conversionBytesParsing(const std::string& str) {
	size_t result = 0;

	std::map<char, size_t> all_convertion;
	all_convertion['k'] = 1024;
	all_convertion['m'] = 1048576;
	all_convertion['g'] = 1073741824;

	result = std::strtoul(str.c_str(), NULL, 10);

	if (result == std::numeric_limits<size_t>::max() || result == 0) {
		throw std::exception();
	}

	std::map<char, size_t>::iterator it_value = all_convertion.find(str.at(str.size() - 1));

	if (it_value != all_convertion.end()) {
		result *= it_value->second;
	} else if (str.at(str.size() - 1) < '0' && str.at(str.size() - 1) > '9') {
		throw std::exception();
	}
	return result;
}

// std::string getExtension(const std::string& path) {
// 	size_t slashPos = path.find_last_of('/');
// 	size_t dotPos = path.find_last_of('.');
// 	if (dotPos == std::string::npos || (slashPos != std::string::npos && dotPos < slashPos)) return "";

// 	return path.substr(dotPos);
// }

// std::string addPath(const std::string& base, const std::string& suffix) {
// 	if (base.empty()) {
// 		if (!suffix.empty() && suffix[0] != '/') return std::string("/") + suffix;
// 		return suffix;
// 	}

// 	if (suffix.empty()) return base;

// 	const bool baseEndsWithSlash = base[base.size() - 1] == '/';
// 	const bool suffixStartsWithSlash = suffix[0] == '/';

// 	if (baseEndsWithSlash && suffixStartsWithSlash) return base + suffix.substr(1);

// 	if (!baseEndsWithSlash && !suffixStartsWithSlash) return base + "/" + suffix;

// 	return base + suffix;
// }

std::string trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\r\n");
	size_t end = str.find_last_not_of(" \t\r\n");
	if (start == std::string::npos) return "";
	return str.substr(start, end - start + 1);
}
