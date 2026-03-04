/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/04 22:08:20 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/GlobalConfig.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <stdexcept>

#include "../../include/config/ServerConfig.hpp"

typedef std::map<std::string,
				 void (GlobalConfig::*)(const std::list<std::string>& words)>
	map_handler;

GlobalConfig::GlobalConfig(const std::string& pathConfigFile) {
	parseConfigFile(readConfigFile(pathConfigFile));
}

GlobalConfig::GlobalConfig(const GlobalConfig& src)
	: Config(src), server(src.server) {}

GlobalConfig::~GlobalConfig() {
	for (std::map<int, ServerConfig*>::iterator it = server.begin();
		 it != server.end(); it++) {
		delete it->second;
	}
}

GlobalConfig& GlobalConfig::operator=(const GlobalConfig& rhs) {
	if (this != &rhs) {
		Config::operator=(rhs);
		server = rhs.server;
	}
	return *this;
}

std::string GlobalConfig::readConfigFile(const std::string& pathConfigFile) {
	std::string line;
	std::string all_directive;
	std::ifstream ifs(pathConfigFile.c_str());

	if (ifs.fail()) {
		throw std::runtime_error("Error: Unable to open config file '" +
								 pathConfigFile + "'");
	}
	while (true) {
		if (!std::getline(ifs, line)) {
			return all_directive;
		}
		if (ifs.fail()) {
			throw std::runtime_error("Error: Failed to read config file '" +
									 pathConfigFile + "'");
		}
		all_directive += line + "\n";
	}
}

void GlobalConfig::parseConfigFile(const std::string& allDirective) {
	map_handler all_handler;
	all_handler["root"] = &GlobalConfig::handleRoot;
	all_handler["index"] = &GlobalConfig::handleIndex;
	all_handler["error_page"] = &GlobalConfig::handleErrorPage;
	all_handler["autoindex"] = &GlobalConfig::handleAutoIndex;
	all_handler["client_max_body_size"] =
		&GlobalConfig::handleClientMaxBodySize;
	all_handler["large_client_header_buffers"] =
		&GlobalConfig::handleLargeClientHeaderBuffers;
	all_handler["client_header_buffer_size"] =
		&GlobalConfig::handleClientHeaderBufferSize;
	all_handler["return"] = &GlobalConfig::handleRedirection;
	all_handler["upload_store"] = &GlobalConfig::handleUploadStore;

	std::string content = allDirective;
	std::list<std::string> words;
	int bracket_count = 0;
	size_t pos = 0;

	size_t idx = 0;
	while (idx < content.length()) {
		if (content[idx] == '#') {
			size_t newline = content.find('\n', idx);
			if (newline == std::string::npos) {
				content.erase(idx);
				break;
			}
			content.erase(idx, newline - idx);
		} else {
			idx++;
		}
	}

	while (pos < content.length()) {
		while (pos < content.length() &&
			   (content[pos] == ' ' || content[pos] == '\t' ||
				content[pos] == '\n' || content[pos] == '\r')) {
			pos++;
		}

		if (pos >= content.length()) {
			break;
		}

		if (content[pos] == '{') {
			bracket_count++;
			pos++;
			continue;
		}

		if (content[pos] == '}') {
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
		while (pos < content.length() && content[pos] != ' ' &&
			   content[pos] != '\t' && content[pos] != '\n' &&
			   content[pos] != '\r' && content[pos] != ';' &&
			   content[pos] != '{' && content[pos] != '}') {
			pos++;
		}

		if (pos > start) {
			std::string token = content.substr(start, pos - start);

			if (token == "server") {
				continue;
			}

			words.push_back(token);
		}

		while (pos < content.length() &&
			   (content[pos] == ' ' || content[pos] == '\t' ||
				content[pos] == '\n' || content[pos] == '\r')) {
			pos++;
		}

		if (pos < content.length() && content[pos] == ';') {
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

	handleServer(content);
}

void GlobalConfig::handleServer(const std::string& allDirective) {
	size_t pos = 0;

	while ((pos = allDirective.find("server", pos)) != std::string::npos) {
		try {
			pos += 6;

			while (pos < allDirective.length() &&
				   (allDirective[pos] == ' ' || allDirective[pos] == '\t' ||
					allDirective[pos] == '\n' || allDirective[pos] == '\r')) {
				pos++;
			}

			if (pos >= allDirective.length() || allDirective[pos] != '{') {
				throw std::runtime_error(
					"Error: Expected '{' after server keyword");
			}
			size_t block_start = pos + 1;

			int brace_count = 1;
			pos++;
			while (pos < allDirective.length() && brace_count > 0) {
				if (allDirective[pos] == '{') {
					brace_count++;
				} else if (allDirective[pos] == '}') {
					brace_count--;
				}
				if (brace_count > 0) {
					pos++;
				}
			}

			if (brace_count != 0) {
				throw std::runtime_error(
					"Error: Unmatched braces in server block");
			}

			std::string server_content =
				allDirective.substr(block_start, pos - block_start);

			ServerConfig* server_ptr = new ServerConfig(server_content, *this);

			if (server.find(server_ptr->port) != server.end()) {
				delete server_ptr;
				throw std::runtime_error(
					"Error: Duplicate server port detected");
			}

			server[server_ptr->port] = server_ptr;

			pos++;
		} catch (const std::exception& e) {
			for (std::map<int, ServerConfig*>::iterator it = server.begin();
				 it != server.end(); ++it) {
				delete it->second;
			}
			server.clear();
			throw;
		}
	}
}

void GlobalConfig::printDirectives() const {
	std::cout << "=== Global Configuration ===\n";

	std::cout << "Host: " << host << '\n';
	std::cout << "Port: " << port << '\n';
	std::cout << "Root: " << root << '\n';

	std::cout << "Index: ";
	for (std::vector<std::string>::const_iterator it = index.begin();
		 it != index.end(); ++it) {
		if (it != index.begin()) {
			std::cout << ", ";
		}
		std::cout << *it;
	}
	std::cout << '\n';

	std::cout << "Autoindex: " << (autoindex ? "on" : "off") << '\n';
	std::cout << "Client Max Body Size: " << client_max_body_size << '\n';

	std::cout << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = error_pages.begin();
		 it != error_pages.end(); ++it) {
		std::cout << "  " << it->first << " -> " << it->second << '\n';
	}

	std::cout << "Methods: ";
	for (std::vector<std::string>::const_iterator it = methods.begin();
		 it != methods.end(); ++it) {
		if (it != methods.begin()) {
			std::cout << ", ";
		}
		std::cout << *it;
	}
	std::cout << '\n';

	if (redirection.first != 0) {
		std::cout << "Redirection: " << redirection.first << " -> "
				  << redirection.second << '\n';
	}

	std::cout << "Large Client Header Buffers: " << large_client_header_buffers
			  << '\n';
	std::cout << "Client Header Buffer Size: " << client_header_buffer_size
			  << '\n';
	std::cout << "Upload Store: " << upload_store << '\n';

	std::cout << "\n=== Servers (" << server.size() << ") ===\n";

	for (std::map<int, ServerConfig*>::const_iterator srv_it = server.begin();
		 srv_it != server.end(); ++srv_it) {
		std::cout << "\n--- Server on port " << srv_it->first << " ---\n";
		const ServerConfig* srv = srv_it->second;

		std::cout << "  Host: " << srv->host << '\n';
		std::cout << "  Port: " << srv->port << '\n';
		std::cout << "  Root: " << srv->root << '\n';

		std::cout << "  Index: ";
		for (std::vector<std::string>::const_iterator it = srv->index.begin();
			 it != srv->index.end(); ++it) {
			if (it != srv->index.begin()) {
				std::cout << ", ";
			}
			std::cout << *it;
		}
		std::cout << '\n';

		std::cout << "  Autoindex: " << (srv->autoindex ? "on" : "off") << '\n';
		std::cout << "  Client Max Body Size: " << srv->client_max_body_size
				  << '\n';

		if (!srv->error_pages.empty()) {
			std::cout << "  Error Pages:\n";
			for (std::map<int, std::string>::const_iterator it =
					 srv->error_pages.begin();
				 it != srv->error_pages.end(); ++it) {
				std::cout << "    " << it->first << " -> " << it->second
						  << '\n';
			}
		}

		std::cout << "  Methods: ";
		for (std::vector<std::string>::const_iterator it = srv->methods.begin();
			 it != srv->methods.end(); ++it) {
			if (it != srv->methods.begin()) {
				std::cout << ", ";
			}
			std::cout << *it;
		}
		std::cout << '\n';

		if (srv->redirection.first != 0) {
			std::cout << "  Redirection: " << srv->redirection.first << " -> "
					  << srv->redirection.second << '\n';
		}

		std::cout << "  Upload Store: " << srv->upload_store << '\n';

		// Print locations
		if (!srv->location.empty()) {
			std::cout << "\n  === Locations (" << srv->location.size()
					  << ") ===\n";
			for (std::map<std::string, Config*>::const_iterator loc_it =
					 srv->location.begin();
				 loc_it != srv->location.end(); ++loc_it) {
				std::cout << "\n  --- Location: " << loc_it->first << " ---\n";
				const Config* loc = loc_it->second;

				std::cout << "    Root: " << loc->root << '\n';

				std::cout << "    Index: ";
				for (std::vector<std::string>::const_iterator it =
						 loc->index.begin();
					 it != loc->index.end(); ++it) {
					if (it != loc->index.begin()) {
						std::cout << ", ";
					}
					std::cout << *it;
				}
				std::cout << '\n';

				std::cout << "    Autoindex: "
						  << (loc->autoindex ? "on" : "off") << '\n';
				std::cout << "    Client Max Body Size: "
						  << loc->client_max_body_size << '\n';

				if (!loc->error_pages.empty()) {
					std::cout << "    Error Pages:\n";
					for (std::map<int, std::string>::const_iterator it =
							 loc->error_pages.begin();
						 it != loc->error_pages.end(); ++it) {
						std::cout << "      " << it->first << " -> "
								  << it->second << '\n';
					}
				}

				std::cout << "    Methods: ";
				for (std::vector<std::string>::const_iterator it =
						 loc->methods.begin();
					 it != loc->methods.end(); ++it) {
					if (it != loc->methods.begin()) {
						std::cout << ", ";
					}
					std::cout << *it;
				}
				std::cout << '\n';

				if (loc->redirection.first != 0) {
					std::cout << "    Redirection: " << loc->redirection.first
							  << " -> " << loc->redirection.second << '\n';
				}

				std::cout << "    Upload Store: " << loc->upload_store << '\n';
			}
		}
	}

	std::cout << "\n===========================\n";
}
