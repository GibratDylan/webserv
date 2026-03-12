/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PathUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 22:02:25 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/12 22:35:17 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/PathUtils.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "../../include/utility/Logger.hpp"

/*
** --------------------------------- METHODS ----------------------------------
*/

std::string PathUtils::normalize(const std::string& path) {
	// std::vector<std::string> parts;
	// std::stringstream ss(path);
	// std::string item;

	// while (std::getline(ss, item, '/')) {
	// 	if (item == "" || item == ".") {
	// 		continue;
	// 	}

	// 	if (item == "..") {
	// 		if (!parts.empty()) {
	// 			parts.pop_back();
	// 		}
	// 	} else {
	// 		parts.push_back(item);
	// 	}
	// }

	// std::string result;
	// for (size_t i = 0; i < parts.size(); ++i) {
	// 	result += parts[i];
	// 	if (i + 1 < parts.size()) {
	// 		result += "/";
	// 	}
	// }

	// Logger::debug(" Normalize result: " + result);
	// Logger::debug(" Normalize location_path: " + location_path);

	// if (!location_path.empty() && result.compare(0, location_path.length() - 1, location_path, 1, location_path.length()) == 0) {
	// 	result = result.substr(location_path.length() - 1);
	// 	if (result.empty() || result[0] != '/') {
	// 		result = "/" + result;
	// 	}
	// }

	// Logger::debug(" Normalized path: " + result);

	// return result;
}

std::string PathUtils::join(const std::string& base, const std::string& relative) {}

std::string PathUtils::getExtension(const std::string& path) {}

bool PathUtils::isSafe(const std::string& path) {}

/* ************************************************************************** */
