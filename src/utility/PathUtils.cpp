/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PathUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 22:02:25 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/16 09:43:55 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/PathUtils.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "../../include/utility/Logger.hpp"

std::string PathUtils::normalize(const std::string& path) {
	std::vector<std::string> parts;
	std::stringstream str_stream(path);
	std::string item;

	if (!path.empty() && path.at(0) == '.') {
		parts.push_back(".");
	}

	while (std::getline(str_stream, item, '/') != NULL) {
		if (!item.empty() && item != "." && item != "..") {
			parts.push_back(item);
		} else if (item == ".." && !parts.empty()) {
			parts.pop_back();
		}
	}

	std::string result;
	for (std::vector<std::string>::iterator parts_it = parts.begin(); parts_it != parts.end(); parts_it++) {
		result += *parts_it;
		if (parts_it < --parts.end()) {
			result += "/";
		}
	}

	return result;
}

std::string PathUtils::resolve(const std::string& base, const std::string& relative) {
	std::string result;
	std::string normalized_base = normalize(base);
	std::string normalized_relative = normalize(relative);

	Logger::debug(" Resolved normalized_base: " + normalized_base);
	Logger::debug(" Resolved normalized_relative: " + normalized_relative);

	if (!normalized_relative.empty() && normalized_base.compare(0, normalized_relative.length(), normalized_relative) == 0) {
		result = normalized_base.substr(normalized_relative.length());
	}

	Logger::debug(" Resolved path: " + result);
	return result;
}

std::string PathUtils::join(const std::string& base, const std::string& relative) {
	if (base.empty()) return normalize(relative);
	if (relative.empty()) return normalize(base);
	return normalize(base + "/" + relative);
}

std::string PathUtils::getExtension(const std::string& path) {
	size_t slash_pos = path.find_last_of('/');
	size_t dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos || (slash_pos != std::string::npos && dot_pos < slash_pos)) {
		Logger::info(" No extension found in: " + path);
		return "";
	}

	return path.substr(dot_pos);
}
