/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/10 20:57:08 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/GlobalConfig.hpp"

#include <cstdlib>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>

#include "../../include/config/ServerConfig.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utility/TResourceGard.hpp"

typedef std::map<std::string, void (GlobalConfig::*)(const std::list<std::string>& words)> map_handler;

GlobalConfig::GlobalConfig(const std::string& pathConfigFile) {
	std::string content = FileSystem::readFile(pathConfigFile);
	parseGlobalDirective(content);
}

GlobalConfig::GlobalConfig(const GlobalConfig& src) : Config(src), server(src.server) {}

GlobalConfig::~GlobalConfig() {}

GlobalConfig& GlobalConfig::operator=(const GlobalConfig& rhs) {
	if (this != &rhs) {
		Config::operator=(rhs);
		server = rhs.server;
	}
	return *this;
}

void GlobalConfig::parseGlobalDirective(const std::string& allDirective) {
	map_handler all_handler;
	all_handler["root"] = &GlobalConfig::handleRoot;
	all_handler["index"] = &GlobalConfig::handleIndex;
	all_handler["error_page"] = &GlobalConfig::handleErrorPage;
	all_handler["autoindex"] = &GlobalConfig::handleAutoIndex;
	all_handler["max_connections"] = &GlobalConfig::handleMaxConnections;
	all_handler["session_timeout"] = &GlobalConfig::handleSessionTimeout;
	all_handler["client_max_body_size"] = &GlobalConfig::handleClientMaxBodySize;
	all_handler["large_client_header_buffers"] = &GlobalConfig::handleLargeClientHeaderBuffers;
	all_handler["client_header_buffer_size"] = &GlobalConfig::handleClientHeaderBufferSize;

	std::string content = allDirective;
	std::list<std::string> words;
	size_t pos = 0;
	bool server_already_pass = false;

	size_t count_double_quotes = 0;
	size_t idx = 0;
	while (idx < content.length()) {
		if (content[idx] == '"') {
			count_double_quotes++;
		}
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

	if (count_double_quotes % 2) {
		throw std::runtime_error("Error: Double quotes not closed");
	}

	while (pos < content.length()) {
		pos = content.find_first_not_of(" \t\n\r", pos);
		if (pos == std::string::npos) {
			break;
		}

		size_t start = pos;
		pos = content.find_first_of(" \t\n\r;}{", start);
		if (pos == std::string::npos) {
			pos = content.length();
		}

		if (content[pos] == '}' || content[pos] == '{') {
			throw std::runtime_error("Error: Bracket without directive in global");
		}

		if (pos > start) {
			words.push_back(content.substr(start, pos - start));
		}

		pos = content.find_first_not_of(" \t\n\r", pos);
		if (pos == std::string::npos) {
			pos = content.length();
		}

		if (pos < content.length() && (content[pos] == ';' || content[pos] == '{')) {
			if (content[pos] == ';') {
				pos++;
			}

			if (words.empty()) {
				throw std::runtime_error("Error: Empty directive before semicolon");
			}

			std::string key = words.front();
			words.pop_front();

			map_handler::iterator map_it = all_handler.find(key);
			if ((map_it == all_handler.end() || server_already_pass) && key != "server") {
				throw std::runtime_error("Error: Unknown directive in global '" + key + "'");
			}

			if (key == "server") {
				server_already_pass = true;
				pos += handleServer(content.substr(pos));
			} else {
				(this->*map_it->second)(words);
			}

			words.clear();
		}
	}

	if (!words.empty()) {
		throw std::runtime_error("Error: Directive without semicolon terminator in global");
	}

	if (server.empty()) {
		throw std::runtime_error("Error: Config file need at least one server");
	}

	if (Logger::isDebugEnabled()) {
		Logger::debug(" Parsing config file:\n" + printDirectives());
	}
}

size_t GlobalConfig::handleServer(const std::string& serverDirective) {
	size_t pos = 0;

	if (serverDirective[pos] != '{') {
		throw std::runtime_error("Error: Server require brackets");
	}

	size_t block_start = ++pos;
	int brace_count = 1;

	while (pos < serverDirective.length() && brace_count != 0) {
		if (serverDirective[pos] == '{') {
			brace_count++;
		} else if (serverDirective[pos] == '}') {
			brace_count--;
		}
		pos++;
	}

	if (brace_count != 0) {
		throw std::runtime_error("Error: Unmatched braces in server block");
	}

	std::string server_content = serverDirective.substr(block_start, pos - block_start - 1);

	ServerConfig tmp_server_config(server_content, *this);

	if (server.find(tmp_server_config.port) != server.end()) {
		throw std::runtime_error("Error: Duplicate server port detected");
	}

	server.insert(std::make_pair(tmp_server_config.port, tmp_server_config));

	return pos;
}

std::string GlobalConfig::printDirectives() const {
	std::ostringstream out;
	out << "=== Global Configuration ===\n";

	out << "Host: " << host << '\n';
	out << "Port: " << port << '\n';
	out << "Root: " << root << '\n';

	out << "Index: ";
	for (std::vector<std::string>::const_iterator it = index.begin(); it != index.end(); ++it) {
		if (it != index.begin()) {
			out << ", ";
		}
		out << *it;
	}
	out << '\n';

	out << "Autoindex: " << (autoindex ? "on" : "off") << '\n';
	out << "Client Max Body Size: " << client_max_body_size << '\n';

	out << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
		out << "  " << it->first << " -> " << it->second << '\n';
	}

	out << "Methods: ";
	for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
		if (it != methods.begin()) {
			out << ", ";
		}
		out << *it;
	}
	out << '\n';

	if (redirection.first != 0) {
		out << "Redirection: " << redirection.first << " -> " << redirection.second << '\n';
	}

	out << "Large Client Header Buffers: " << large_client_header_buffers << '\n';
	out << "Client Header Buffer Size: " << client_header_buffer_size << '\n';
	if (!upload_store.empty()) {
		out << "Upload Store: " << upload_store << '\n';
	}

	if (!cgi_handlers.empty()) {
		out << "CGI handlers:\n";
		for (std::map<std::string, std::string>::const_iterator it = cgi_handlers.begin(); it != cgi_handlers.end(); ++it) {
			out << "  " << it->second << " -> " << it->first << '\n';
		}
	}

	out << "Max Connections: " << max_connections << '\n';
	out << "Session timeout: " << session_timeout << '\n';

	out << "\n=== Servers (" << server.size() << ") ===\n";

	for (std::map<int, ServerConfig>::const_iterator srv_it = server.begin(); srv_it != server.end(); ++srv_it) {
		out << "\n--- Server on port " << srv_it->first << " ---\n";
		const ServerConfig srv = srv_it->second;

		out << "  Host: " << srv.host << '\n';
		out << "  Port: " << srv.port << '\n';
		out << "  Root: " << srv.root << '\n';

		out << "  Index: ";
		for (std::vector<std::string>::const_iterator it = srv.index.begin(); it != srv.index.end(); ++it) {
			if (it != srv.index.begin()) {
				out << ", ";
			}
			out << *it;
		}
		out << '\n';

		out << "  Autoindex: " << (srv.autoindex ? "on" : "off") << '\n';
		out << "  Client Max Body Size: " << srv.client_max_body_size << '\n';

		if (!srv.error_pages.empty()) {
			out << "  Error Pages:\n";
			for (std::map<int, std::string>::const_iterator it = srv.error_pages.begin(); it != srv.error_pages.end(); ++it) {
				out << "    " << it->first << " -> " << it->second << '\n';
			}
		}

		out << "  Methods: ";
		for (std::vector<std::string>::const_iterator it = srv.methods.begin(); it != srv.methods.end(); ++it) {
			if (it != srv.methods.begin()) {
				out << ", ";
			}
			out << *it;
		}
		out << '\n';

		if (srv.redirection.first != 0) {
			out << "  Redirection: " << srv.redirection.first << " -> " << srv.redirection.second << '\n';
		}

		out << "  Large Client Header Buffers: " << srv.large_client_header_buffers << '\n';
		out << "  Client Header Buffer Size: " << srv.client_header_buffer_size << '\n';
		if (!srv.upload_store.empty()) {
			out << "  Upload Store: " << srv.upload_store << '\n';
		}

		if (!srv.location.empty()) {
			out << "\n  === Locations (" << srv.location.size() << ") ===\n";
			for (std::map<std::string, Config>::const_iterator loc_it = srv.location.begin(); loc_it != srv.location.end(); ++loc_it) {
				out << "\n  --- Location: " << loc_it->first << " ---\n";
				const Config loc = loc_it->second;

				out << "    Root: " << loc.root << '\n';

				out << "    Index: ";
				for (std::vector<std::string>::const_iterator it = loc.index.begin(); it != loc.index.end(); ++it) {
					if (it != loc.index.begin()) {
						out << ", ";
					}
					out << *it;
				}
				out << '\n';

				out << "    Autoindex: " << (loc.autoindex ? "on" : "off") << '\n';
				out << "    Client Max Body Size: " << loc.client_max_body_size << '\n';

				if (!loc.error_pages.empty()) {
					out << "    Error Pages:\n";
					for (std::map<int, std::string>::const_iterator it = loc.error_pages.begin(); it != loc.error_pages.end(); ++it) {
						out << "      " << it->first << " -> " << it->second << '\n';
					}
				}

				out << "    Methods: ";
				for (std::vector<std::string>::const_iterator it = loc.methods.begin(); it != loc.methods.end(); ++it) {
					if (it != loc.methods.begin()) {
						out << ", ";
					}
					out << *it;
				}
				out << '\n';

				if (loc.redirection.first != 0) {
					out << "    Redirection: " << loc.redirection.first << " -> " << loc.redirection.second << '\n';
				}

				out << "    Large Client Header Buffers: " << loc.large_client_header_buffers << '\n';
				out << "    Client Header Buffer Size: " << loc.client_header_buffer_size << '\n';
				if (!loc.upload_store.empty()) {
					out << "    Upload Store: " << loc.upload_store << '\n';
				}

				if (!loc.cgi_handlers.empty()) {
					out << "    CGI handlers:\n";
					for (std::map<std::string, std::string>::const_iterator it = loc.cgi_handlers.begin(); it != loc.cgi_handlers.end(); ++it) {
						out << "      " << it->second << " -> " << it->first << '\n';
					}
				}

				if (loc.redirection.first) {
					out << "    Return: " << loc.redirection.first << " -> " << loc.redirection.second << '\n';
				}
			}
		}
	}

	out << "\n===========================\n";
	return out.str();
}
