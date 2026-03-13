/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/11 20:22:08 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/ServerConfig.hpp"

#include <cstdlib>
#include <list>
#include <stdexcept>

#include "../../include/utility/Logger.hpp"
#include "../../include/utility/TResourceGard.hpp"
#include "../../include/utils.h"

typedef std::map<std::string, void (ServerConfig::*)(const std::list<std::string>& words)> map_handler;

ServerConfig::ServerConfig(const std::string& serverDirective, const Config& globalConfig) : Config(globalConfig) {
	parseServerDirective(serverDirective);
}

ServerConfig::ServerConfig(const ServerConfig& src) : Config(src), location(src.location) {}

ServerConfig::~ServerConfig() {}

ServerConfig& ServerConfig::operator=(const ServerConfig& rhs) {
	if (this != &rhs) {
		Config::operator=(rhs);
		location = rhs.location;
	}
	return *this;
}

void ServerConfig::parseServerDirective(const std::string& serverDirective) {
	map_handler all_handler;
	all_handler["listen"] = &ServerConfig::handleListen;
	all_handler["root"] = &ServerConfig::handleRoot;
	all_handler["index"] = &ServerConfig::handleIndex;
	all_handler["error_page"] = &ServerConfig::handleErrorPage;
	all_handler["autoindex"] = &ServerConfig::handleAutoIndex;
	all_handler["client_max_body_size"] = &ServerConfig::handleClientMaxBodySize;
	all_handler["large_client_header_buffers"] = &ServerConfig::handleLargeClientHeaderBuffers;
	all_handler["client_header_buffer_size"] = &ServerConfig::handleClientHeaderBufferSize;
	all_handler["return"] = &ServerConfig::handleRedirection;
	all_handler["allow_methods"] = &ServerConfig::handleMethods;
	all_handler["upload_store"] = &ServerConfig::handleUploadStore;

	std::list<std::string> words;
	size_t pos = 0;
	bool location_already_pass = false;

	while (pos < serverDirective.length()) {
		pos = serverDirective.find_first_not_of(" \t\n\r", pos);
		if (pos == std::string::npos) {
			break;
		}

		size_t start = pos;
		if (serverDirective[pos] == '"') {
			pos = serverDirective.find_first_of('"', ++start);
			if (pos == std::string::npos) {
				pos = serverDirective.length();
			}
		} else {
			pos = serverDirective.find_first_of(" \t\n\r;}{", start);
			if (pos == std::string::npos) {
				pos = serverDirective.length();
			}
		}

		if (serverDirective[pos] == '}' || serverDirective[pos] == '{') {
			throw std::runtime_error("Error: Bracket without directive in server");
		}

		if (pos > start) {
			words.push_back(serverDirective.substr(start, pos - start));
		}

		if (serverDirective[pos] == '"') {
			pos++;
		}

		pos = serverDirective.find_first_not_of(" \t\n\r", pos);
		if (pos == std::string::npos) {
			pos = serverDirective.length();
		}

		if (pos < serverDirective.length() && (serverDirective[pos] == ';' || serverDirective[pos] == '{')) {
			if (serverDirective[pos] == ';') {
				pos++;
			}

			if (words.empty()) {
				throw std::runtime_error("Error: Empty directive before semicolon");
			}

			std::string key = words.front();
			words.pop_front();

			map_handler::iterator map_it = all_handler.find(key);
			if ((map_it == all_handler.end() || location_already_pass) && key != "location") {
				throw std::runtime_error("Error: Unknown directive in server '" + key + "'");
			}

			if (key == "location") {
				if (words.size() != 1 && words.size() != 2) {
					throw std::runtime_error("Error: Location requires a path and an optional modifier");
				}
				bool isFile = (words.size() == 2 && words.back() == "FILE");
				if (words.size() == 2 && !isFile) {
					throw std::runtime_error("Error: Unknown location modifier '" + words.back() + "'");
				}
				location_already_pass = true;
				pos += handleLocation(serverDirective.substr(pos), words.front(), isFile);
			} else {
				(this->*map_it->second)(words);
			}

			words.clear();
		}
	}

	if (!words.empty()) {
		Logger::error(" Server directive missing semicolon terminator");
		throw std::runtime_error("Error: Directive without semicolon terminator");
	}

	Logger::debug(std::string(" Server directive parsed on port ") + toString(port));
}

size_t ServerConfig::handleLocation(const std::string& locationDirective, const std::string& pathLocation, bool isFile) {
	size_t pos = 0;

	if (locationDirective[pos] != '{') {
		throw std::runtime_error("Error: Location require brackets");
	}

	size_t block_start = ++pos;
	int brace_count = 1;

	while (pos < locationDirective.length() && brace_count != 0) {
		if (locationDirective[pos] == '{') {
			brace_count++;
		} else if (locationDirective[pos] == '}') {
			brace_count--;
		}
		pos++;
	}

	if (brace_count != 0) {
		throw std::runtime_error("Error: Unmatched braces in location block");
	}

	std::string location_content = locationDirective.substr(block_start, pos - block_start - 1);

	Config tm_config(location_content, *this);
	tm_config.location_path = pathLocation;
	tm_config.isFile = isFile;

	if (location.find(pathLocation) == location.end()) {
		location.insert(std::make_pair(pathLocation, tm_config));
	} else {
		throw std::runtime_error("Error: Duplicate location");
	}

	Logger::debug(std::string(" Local directive parsed for location '") + pathLocation + "'");

	return pos;
}

const Config& ServerConfig::resolveConfig(const std::string& locationPath) const {
	Logger::debug(std::string(" resolveConfig path=") + locationPath);

	std::map<std::string, Config>::const_iterator location_it = location.find(locationPath);
	if (location_it != location.end()) {
		Logger::debug(std::string(" resolveConfig exact match=") + locationPath);

		return location_it->second;
	}

	size_t dotPos = locationPath.rfind('.');
	size_t slashPos = locationPath.rfind('/');
	if (dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos)) {
		std::string extension = locationPath.substr(dotPos);
		std::string wildcardLocation = "*" + extension;
		location_it = location.find(wildcardLocation);
		if (location_it != location.end()) {
			Logger::debug(std::string(" resolveConfig wildcard match=") + wildcardLocation);
			return location_it->second;
		}
	}

	std::string searchPath = locationPath;
	while (true) {
		location_it = location.find(searchPath);
		if (location_it != location.end()) {
			Logger::debug(std::string(" resolveConfig prefix match=") + searchPath);
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
