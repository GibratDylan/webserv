/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/05 23:00:05 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/ServerConfig.hpp"

#include <cstdlib>
#include <list>
#include <stdexcept>

typedef std::map<std::string,
				 void (ServerConfig::*)(const std::list<std::string>& words)>
	map_handler;

ServerConfig::ServerConfig(const ServerConfig& src)
	: Config(src), location(src.location) {}

ServerConfig::~ServerConfig() {
	for (std::map<std::string, Config*>::iterator it = location.begin();
		 it != location.end(); it++) {
		delete it->second;
	}
}

ServerConfig& ServerConfig::operator=(const ServerConfig& rhs) {
	if (this != &rhs) {
		Config::operator=(rhs);
		location = rhs.location;
	}
	return *this;
}

ServerConfig::ServerConfig(const std::string& serverDirective,
						   const Config& globalConfig)
	: Config(globalConfig) {
	try {
		parseServerDirective(serverDirective);
	} catch (const std::exception& e) {
		for (std::map<std::string, Config*>::iterator it = location.begin();
			 it != location.end(); ++it) {
			delete it->second;
		}
		location.clear();
		throw;
	}
}

void ServerConfig::parseServerDirective(const std::string& serverDirective) {
	map_handler all_handler;
	all_handler["listen"] = &ServerConfig::handleListen;
	all_handler["root"] = &ServerConfig::handleRoot;
	all_handler["index"] = &ServerConfig::handleIndex;
	all_handler["error_page"] = &ServerConfig::handleErrorPage;
	all_handler["autoindex"] = &ServerConfig::handleAutoIndex;
	all_handler["client_max_body_size"] =
		&ServerConfig::handleClientMaxBodySize;
	all_handler["large_client_header_buffers"] =
		&ServerConfig::handleLargeClientHeaderBuffers;
	all_handler["client_header_buffer_size"] =
		&ServerConfig::handleClientHeaderBufferSize;
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
			throw std::runtime_error(
				"Error: Bracket without directive in server");
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

		if (pos < serverDirective.length() &&
			(serverDirective[pos] == ';' || serverDirective[pos] == '{')) {
			if (serverDirective[pos] == ';') {
				pos++;
			}

			if (words.empty()) {
				throw std::runtime_error(
					"Error: Empty directive before semicolon");
			}

			std::string key = words.front();
			words.pop_front();

			map_handler::iterator map_it = all_handler.find(key);
			if ((map_it == all_handler.end() || location_already_pass) &&
				key != "location") {
				throw std::runtime_error(
					"Error: Unknown directive in server '" + key + "'");
			}

			if (key == "location") {
				if (words.size() != 1) {
					throw std::runtime_error(
						"Error: Location exactly one path");
				}
				location_already_pass = true;
				pos +=
					handleLocation(serverDirective.substr(pos), words.front());
			} else {
				(this->*map_it->second)(words);
			}

			words.clear();
		}
	}

	if (!words.empty()) {
		throw std::runtime_error(
			"Error: Directive without semicolon terminator");
	}
}

size_t ServerConfig::handleLocation(const std::string& locationDirective,
									const std::string& pathLocation) {
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

	std::string location_content =
		locationDirective.substr(block_start, pos - block_start - 1);

	Config* location_ptr = new Config(location_content, *this);

	if (location.find(pathLocation) == location.end()) {
		location[pathLocation] = location_ptr;
	} else {
		delete location_ptr;
		throw std::runtime_error("Error: Duplicate location");
	}

	return pos;
}

Config ServerConfig::resolveConfig(const std::string& locationPath) const {
	std::map<std::string, Config*>::const_iterator it_location =
		location.find(locationPath);
	if (it_location != location.end()) {
		return *(it_location->second);
	}

	std::string search_path = locationPath;
	while (search_path.length() > 1) {
		size_t last_slash = search_path.rfind('/');
		if (last_slash == std::string::npos || last_slash == 0) {
			break;
		}
		search_path = search_path.substr(0, last_slash + 1);
		it_location = location.find(search_path);
		if (it_location != location.end()) {
			return *(it_location->second);
		}

		search_path = search_path.substr(0, last_slash);
	}
	return *this;
}
