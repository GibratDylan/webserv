/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/04 22:09:07 by dgibrat          ###   ########.fr       */
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
	all_handler["upload_store"] = &ServerConfig::handleUploadStore;

	std::list<std::string> words;
	int bracket_count = 0;
	size_t pos = 0;


	while (pos < serverDirective.length()) {

		while (pos < serverDirective.length() &&
			   (serverDirective[pos] == ' ' || serverDirective[pos] == '\t' ||
				serverDirective[pos] == '\n' || serverDirective[pos] == '\r')) {
			pos++;
		}

		if (pos >= serverDirective.length()) {
			break;
		}

		if (serverDirective[pos] == '{') {
			bracket_count++;
			words.clear();
			
			pos++;
			continue;
		}

		if (serverDirective[pos] == '}') {
			if (bracket_count == 0) {
				throw std::runtime_error(
					"Error: Unexpected closing bracket without matching open "
					"bracket");
			}
			bracket_count--;
			pos++;
			continue;
		}

		if (bracket_count > 0) {
			pos++;
			continue;
		}

		size_t start = pos;
		while (pos < serverDirective.length() && serverDirective[pos] != ' ' &&
			   serverDirective[pos] != '\t' && serverDirective[pos] != '\n' &&
			   serverDirective[pos] != '\r' && serverDirective[pos] != ';' &&
			   serverDirective[pos] != '{' && serverDirective[pos] != '}') {
			pos++;
		}

		if (pos > start) {
			words.push_back(serverDirective.substr(start, pos - start));
		}

		while (pos < serverDirective.length() &&
			   (serverDirective[pos] == ' ' || serverDirective[pos] == '\t' ||
				serverDirective[pos] == '\n' || serverDirective[pos] == '\r')) {
			pos++;
		}

		if (pos < serverDirective.length() && serverDirective[pos] == ';') {
			pos++;

			if (words.empty()) {
				throw std::runtime_error(
					"Error: Empty directive before semicolon");
			}

			std::string key = words.front();
			words.pop_front();

			map_handler::iterator map_it = all_handler.find(key);
			if (map_it == all_handler.end()) {
				throw std::runtime_error("Error: Unknown directive '" + key +
										 "'");
			}
			(this->*map_it->second)(words);
			words.clear();
		}
	}

	if (bracket_count != 0) {
		throw std::runtime_error("Error: Unclosed bracket in configuration");
	}

	if (!words.empty()) {
		throw std::runtime_error(
			"Error: Directive without semicolon terminator");
	}

	handleLocation(serverDirective);
}

void ServerConfig::handleLocation(const std::string& serverDirective) {
	size_t pos = 0;

	while ((pos = serverDirective.find("location", pos)) != std::string::npos) {
		Config* location_ptr = NULL;
		try {
			pos += 8;

			while (pos < serverDirective.length() &&
				   (serverDirective[pos] == ' ' ||
					serverDirective[pos] == '\t' ||
					serverDirective[pos] == '\n' ||
					serverDirective[pos] == '\r')) {
				pos++;
			}

			size_t path_start = pos;
			while (
				pos < serverDirective.length() && serverDirective[pos] != ' ' &&
				serverDirective[pos] != '\t' && serverDirective[pos] != '\n' &&
				serverDirective[pos] != '\r' && serverDirective[pos] != '{') {
				pos++;
			}
			std::string location_path =
				serverDirective.substr(path_start, pos - path_start);

			while (pos < serverDirective.length() &&
				   (serverDirective[pos] == ' ' ||
					serverDirective[pos] == '\t' ||
					serverDirective[pos] == '\n' ||
					serverDirective[pos] == '\r')) {
				pos++;
			}

			if (pos >= serverDirective.length() ||
				serverDirective[pos] != '{') {
				throw std::runtime_error(
					"Error: Expected '{' after location path");
			}
			size_t block_start = pos + 1;
			
			int brace_count = 1;
			pos++;
			while (pos < serverDirective.length() && brace_count > 0) {
				if (serverDirective[pos] == '{') {
					brace_count++;
				} else if (serverDirective[pos] == '}') {
					brace_count--;
				}
				if (brace_count > 0) {
					pos++;
				}
			}

			if (brace_count != 0) {
				throw std::runtime_error(
					"Error: Unmatched braces in location block");
			}

			std::string location_content =
				serverDirective.substr(block_start, pos - block_start);

			location_ptr = new Config(location_content, *this);

			if (location.find(location_path) == location.end()) {
				location[location_path] = location_ptr;
			} else {
				delete location_ptr;
				throw std::runtime_error("Error: Duplicate location");
			}

			pos++;
		} catch (const std::exception& e) {
			for (std::map<std::string, Config*>::iterator it = location.begin();
				 it != location.end(); ++it) {
				delete it->second;
			}
			location.clear();
			throw;
		}
	}
}

// const std::string& ServerConfig::getDirective(const std::string& interface,
// 											  const std::string& location_path,
// 											  const std::string& name) const {
// 	// TODO: Implement directive lookup
// 	(void)interface;
// 	(void)location_path;
// 	(void)name;
// 	static std::string empty;
// 	return empty;
// }

Config ServerConfig::resolveConfig(const std::string& locationPath) const {
	std::map<std::string, Config*>::const_iterator it = location.find(locationPath);
	if (it != location.end()) 
		return *(it->second); 

	std::string searchPath = locationPath;
	while (searchPath.length() > 1) 
	{
		size_t lastSlash = searchPath.rfind('/');
		if (lastSlash == std::string::npos || lastSlash == 0) 
			break;
		searchPath = searchPath.substr(0, lastSlash + 1);
		it = location.find(searchPath);
		if (it != location.end()) 
			return *(it->second);

		searchPath = searchPath.substr(0, lastSlash);
	}
	return *this;
}
