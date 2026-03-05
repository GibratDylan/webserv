#include <cctype>
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

	std::map<char, size_t>::iterator it_value =
		all_convertion.find(str.at(str.size() - 1));

	if (it_value != all_convertion.end()) {
		result *= it_value->second;
	} else if (str.at(str.size() - 1) < '0' && str.at(str.size() - 1) > '9') {
		throw std::exception();
	}
	return result;
}
