/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 11:31:38 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/05 22:44:04 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/Config.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "../../include/http/HttpStatus.hpp"
#include "../../include/utils.h"

typedef std::map<std::string,
				 void (Config::*)(const std::list<std::string>& words)>
	map_handler;

Config::Config()
	: host("localhost"),
	  port(8080),
	  root("./www"),
	  autoindex(false),
	  client_max_body_size(10000000),
	  large_client_header_buffers(8192),
	  client_header_buffer_size(8192) {
	index.push_back("index.html");
	methods.push_back("GET");
	methods.push_back("POST");
	redirection.first = 0;
}

Config::Config(const Config& src)
	: host(src.host),
	  port(src.port),
	  root(src.root),
	  index(src.index),
	  autoindex(src.autoindex),
	  client_max_body_size(src.client_max_body_size),
	  error_pages(src.error_pages),
	  methods(src.methods),
	  redirection(src.redirection),
	  large_client_header_buffers(src.large_client_header_buffers),
	  client_header_buffer_size(src.client_header_buffer_size),
	  upload_store(src.upload_store),
	  cgi(src.cgi) {}

Config::~Config() {}

Config& Config::operator=(const Config& rhs) {
	if (this != &rhs) {
		host = rhs.host;
		port = rhs.port;
		root = rhs.root;
		index = rhs.index;
		autoindex = rhs.autoindex;
		client_max_body_size = rhs.client_max_body_size;
		error_pages = rhs.error_pages;
		methods = rhs.methods;
		redirection = rhs.redirection;
		large_client_header_buffers = rhs.large_client_header_buffers;
		client_header_buffer_size = rhs.client_header_buffer_size;
		upload_store = rhs.upload_store;
		cgi = rhs.cgi;
	}
	return *this;
}

Config::Config(const std::string& localDirective, const Config& serverConfig)
	: host(serverConfig.host),
	  port(serverConfig.port),
	  root(serverConfig.root),
	  index(serverConfig.index),
	  autoindex(serverConfig.autoindex),
	  client_max_body_size(serverConfig.client_max_body_size),
	  error_pages(serverConfig.error_pages),
	  methods(serverConfig.methods),
	  redirection(serverConfig.redirection),
	  large_client_header_buffers(serverConfig.large_client_header_buffers),
	  client_header_buffer_size(serverConfig.client_header_buffer_size),
	  upload_store(serverConfig.upload_store),
	  cgi(serverConfig.cgi) {
	parseLocalDirective(localDirective);
}

void Config::parseLocalDirective(const std::string& localDirective) {
	map_handler all_handler;
	all_handler["root"] = &Config::handleRoot;
	all_handler["index"] = &Config::handleIndex;
	all_handler["error_page"] = &Config::handleErrorPage;
	all_handler["autoindex"] = &Config::handleAutoIndex;
	all_handler["client_max_body_size"] = &Config::handleClientMaxBodySize;
	all_handler["return"] = &Config::handleRedirection;
	all_handler["upload_store"] = &Config::handleUploadStore;
	all_handler["allow_methods"] = &Config::handleMethods;
	all_handler["cgi"] = &Config::handleCGI;

	std::list<std::string> words;
	size_t pos = 0;

	while (pos < localDirective.length()) {
		pos = localDirective.find_first_not_of(" \t\n\r", pos);
		if (pos == std::string::npos) {
			break;
		}

		size_t start = pos;
		if (localDirective[pos] == '"') {
			pos = localDirective.find_first_of('"', ++start);
			if (pos == std::string::npos) {
				pos = localDirective.length();
			}
		} else {
			pos = localDirective.find_first_of(" \t\n\r;}{", start);
			if (pos == std::string::npos) {
				pos = localDirective.length();
			}
		}

		if (localDirective[pos] == '}' || localDirective[pos] == '{') {
			throw std::runtime_error("Error: Bracket in location");
		}

		if (pos > start) {
			words.push_back(localDirective.substr(start, pos - start));
		}

		if (localDirective[pos] == '"') {
			pos++;
		}
		
		pos = localDirective.find_first_not_of(" \t\n\r", pos);
		if (pos == std::string::npos) {
			pos = localDirective.length();
		}

		if (pos < localDirective.length() && localDirective[pos] == ';') {
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

	if (!words.empty()) {
		std::cout << words.front() << "\n";
		throw std::runtime_error(
			"Error: Directive without semicolon terminator");
	}
}

void Config::handleUploadStore(const std::list<std::string>& words) {
	if (words.size() != 1) {
		throw std::runtime_error(
			"Error: 'upload_store' directive requires exactly one argument");
	}

	this->upload_store = words.front();
}

void Config::handleMethods(const std::list<std::string>& words) {
	if (words.size() < 1 && words.size() > 3) {
		throw std::runtime_error(
			"Error: 'allow_methods' directive requires at least one argument "
			"and maximum three");
	}

	std::vector<std::string> valid_methods;
	valid_methods.push_back("GET");
	valid_methods.push_back("POST");
	valid_methods.push_back("DELETE");

	std::list<std::string>::const_iterator list_it = words.begin();

	while (list_it != words.end()) {
		if (std::find(valid_methods.begin(), valid_methods.end(), *list_it) ==
			valid_methods.end()) {
			throw std::runtime_error(
				"Error: 'allow_methods' directive requires GET, POST or "
				"DELETE");
		}
		list_it++;
	}

	this->methods.clear();
	this->methods.insert(this->methods.begin(), words.begin(), words.end());
}

void Config::handleRedirection(const std::list<std::string>& words) {
	if (words.size() < 1 || words.size() > 3) {
		throw std::runtime_error(
			"Error: 'return' directive requires exactly 1 or 2 argument");
	}

	int code = static_cast<int>(std::strtol((words.front()).c_str(), NULL, 10));

	if (!HttpStatus::isValid(code)) {
		throw std::runtime_error(
			"Error: Invalid error code in 'return' directive");
	}

	this->redirection.first = code;
	if (words.size() == 2) {
		this->redirection.second = words.back();
	}
}

void Config::handleListen(const std::list<std::string>& words) {
	if (words.size() != 1) {
		throw std::runtime_error(
			"Error: 'listen' directive requires exactly one argument");
	}

	size_t pos_colon = words.front().find(":");

	if (pos_colon != std::string::npos) {
		this->host = words.front().substr(0, pos_colon);
		this->port = static_cast<int>(
			std::strtol(words.front().substr(pos_colon + 1).c_str(), NULL, 10));
	} else {
		this->port =
			static_cast<int>(std::strtol(words.front().c_str(), NULL, 10));
	}

	if (this->port <= 0 || this->port > 65535) {
		throw std::runtime_error(
			"Error: Invalid port number (must be between 1 and 65535)");
	}
}

void Config::handleCGI(const std::list<std::string>& words) {
	if (words.size() != 1) {
		throw std::runtime_error(
			"Error: 'root' directive requires exactly one argument");
	}

	std::map<std::string, std::string> all_cgi;
	all_cgi["/usr/bin/php-cgi"] = ".php";
	all_cgi["/usr/bin/python3"] = ".py";
	all_cgi["/usr/bin/python2"] = ".py";
	all_cgi["/usr/bin/perl"] = ".pl";
	all_cgi["/usr/bin/ruby"] = ".rb";
	all_cgi["/bin/bash"] = ".sh";
	all_cgi["/usr/bin/node"] = ".js";

	if (all_cgi.find(words.front()) == all_cgi.end()) {
		throw std::runtime_error(
			"Error: 'cgi' directive requires .php/.py/.pl/.rb/.sh/.js");
	}

	this->cgi.first = words.front();
	this->cgi.second = all_cgi[this->cgi.first];
}

void Config::handleRoot(const std::list<std::string>& words) {
	if (words.size() != 1) {
		throw std::runtime_error(
			"Error: 'root' directive requires exactly one argument");
	}

	this->root = words.front();
}

void Config::handleIndex(const std::list<std::string>& words) {
	this->index.clear();
	this->index.insert(this->index.begin(), words.begin(), words.end());
}

void Config::handleErrorPage(const std::list<std::string>& words) {
	if (words.size() < 2) {
		throw std::runtime_error(
			"Error: 'error_page' directive requires at least code and path "
			"arguments");
	}

	std::list<std::string>::const_iterator list_it_code = words.begin();

	this->error_pages.clear();

	while (list_it_code != --words.end()) {
		int code =
			static_cast<int>(std::strtol((*list_it_code).c_str(), NULL, 10));

		if (!HttpStatus::isClientError(code) &&
			!HttpStatus::isServerError(code)) {
			throw std::runtime_error(
				"Error: Invalid error code in 'error_page' directive");
		}

		this->error_pages[code] = words.back();
		list_it_code++;
	}
}

void Config::handleAutoIndex(const std::list<std::string>& words) {
	if (words.size() != 1 ||
		(words.front() != "off" && words.front() != "on")) {
		throw std::runtime_error(
			"Error: 'autoindex' directive requires exactly one argument ('on' "
			"or 'off')");
	}

	this->autoindex = words.front() == "on";
}

void Config::handleClientMaxBodySize(const std::list<std::string>& words) {
	if (words.size() != 1) {
		throw std::runtime_error(
			"Error: 'client_max_body_size' directive requires exactly one "
			"argument");
	}

	try {
		this->client_max_body_size = conversionBytesParsing(words.front());
	} catch (const std::exception& e) {
		throw std::runtime_error(
			"Error: Invalid value for 'client_max_body_size' directive");
	}
}

void Config::handleLargeClientHeaderBuffers(
	const std::list<std::string>& words) {
	if (words.size() != 1) {
		throw std::runtime_error(
			"Error: 'large_client_header_buffers' directive requires exactly "
			"one argument");
	}

	try {
		this->large_client_header_buffers =
			conversionBytesParsing(words.front());
	} catch (const std::exception& e) {
		throw std::runtime_error(
			"Error: Invalid value for 'large_client_header_buffers' directive");
	}
}

void Config::handleClientHeaderBufferSize(const std::list<std::string>& words) {
	if (words.size() != 1) {
		throw std::runtime_error(
			"Error: 'client_header_buffer_size' directive requires exactly one "
			"argument");
	}

	try {
		this->client_header_buffer_size = conversionBytesParsing(words.front());
	} catch (const std::exception& e) {
		throw std::runtime_error(
			"Error: Invalid value for 'client_header_buffer_size' directive");
	}
}
