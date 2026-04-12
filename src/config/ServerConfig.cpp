/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:12 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/ServerConfig.hpp"

#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"

ServerConfig::ServerConfig(const Config& globalConfig)
	: Config(globalConfig), location() {}

ServerConfig::ServerConfig(const ServerConfig& src)
	: Config(src), location(src.location) {}

ServerConfig::~ServerConfig() {}

ServerConfig& ServerConfig::operator=(const ServerConfig& rhs) {
	if (this != &rhs) {
		Config::operator=(rhs);
		location = rhs.location;
	}
	return *this;
}

const Config& ServerConfig::resolveConfig(
	const std::string& locationPath) const {
	Logger::debug(std::string(" resolveConfig path=") + locationPath);

	std::map<std::string, Config>::const_iterator location_it =
		location.find(locationPath);
	if (location_it != location.end()) {
		Logger::debug(std::string(" resolveConfig exact match=") +
					  locationPath);
		return location_it->second;
	}

	size_t dotPos = locationPath.rfind('.');
	size_t slashPos = locationPath.rfind('/');
	if (dotPos != std::string::npos &&
		(slashPos == std::string::npos || dotPos > slashPos)) {
		std::string extension = locationPath.substr(dotPos);
		std::string wildcardLocation = "*" + extension;
		location_it = location.find(wildcardLocation);
		if (location_it != location.end()) {
			Logger::debug(std::string(" resolveConfig wildcard match=") +
						  wildcardLocation);
			return location_it->second;
		}
	}

	std::string searchPath = locationPath.empty() ? "/" : locationPath;
	while (true) {
		location_it = location.find(searchPath);
		if (location_it != location.end()) {
			Logger::debug(std::string(" resolveConfig prefix match=") +
						  searchPath);
			return location_it->second;
		}

		if (searchPath == "/") {
			break;
		}

		size_t lastSlash = searchPath.rfind('/');
		if (lastSlash == std::string::npos) {
			break;
		}
		if (lastSlash == 0) {
			searchPath = "/";
		} else {
			searchPath = searchPath.substr(0, lastSlash);
		}
	}
	Logger::debug(" resolveConfig using server defaults");
	return *this;
}
