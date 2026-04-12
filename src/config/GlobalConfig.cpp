/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:12 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/GlobalConfig.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "../../include/config/ConfigParser.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/utility/Logger.hpp"

GlobalConfig::GlobalConfig() : Config(), server() {}

GlobalConfig::GlobalConfig(const std::string& pathConfigFile)
	: Config(), server() {
	ConfigParser parser;
	GlobalConfig* parsed = NULL;
	try {
		parsed = parser.parse(pathConfigFile);
		*this = *parsed;
		delete parsed;
	} catch (...) {
		delete parsed;
		throw;
	}
}

GlobalConfig::GlobalConfig(const GlobalConfig& src)
	: Config(src), server(src.server) {}

GlobalConfig::~GlobalConfig() {}

GlobalConfig& GlobalConfig::operator=(const GlobalConfig& rhs) {
	if (this != &rhs) {
		Config::operator=(rhs);
		server = rhs.server;
	}
	return *this;
}

std::string GlobalConfig::printDirectives() const {
	std::ostringstream out;
	out << "=== Global Configuration ===\n";

	out << "Host: " << host << '\n';
	out << "Port: " << port << '\n';
	out << "Root: " << root << '\n';

	out << "Index: ";
	for (std::vector<std::string>::const_iterator it = index.begin();
		 it != index.end(); ++it) {
		if (it != index.begin()) {
			out << ", ";
		}
		out << *it;
	}
	out << '\n';

	out << "Autoindex: " << (autoindex ? "on" : "off") << '\n';
	out << "Client Max Body Size: " << client_max_body_size << '\n';

	out << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = error_pages.begin();
		 it != error_pages.end(); ++it) {
		out << "  " << it->first << " -> " << it->second << '\n';
	}

	out << "Methods: ";
	for (std::vector<std::string>::const_iterator it = methods.begin();
		 it != methods.end(); ++it) {
		if (it != methods.begin()) {
			out << ", ";
		}
		out << *it;
	}
	out << '\n';

	if (redirection.first != 0) {
		out << "Redirection: " << redirection.first << " -> "
			<< redirection.second << '\n';
	}

	out << "Large Client Header Buffers: " << large_client_header_buffers
		<< '\n';
	out << "Client Header Buffer Size: " << client_header_buffer_size << '\n';
	if (!upload_store.empty()) {
		out << "Upload Store: " << upload_store << '\n';
	}

	if (!cgi_handlers.empty()) {
		out << "CGI handlers:\n";
		for (std::map<std::string, std::string>::const_iterator it =
				 cgi_handlers.begin();
			 it != cgi_handlers.end(); ++it) {
			out << "  " << it->second << " -> " << it->first << '\n';
		}
	}

	out << "Max Connections: " << max_connections << '\n';
	out << "Session timeout: " << session_timeout << '\n';

	out << "\n=== Servers (" << server.size() << ") ===\n";

	for (std::map<int, ServerConfig>::const_iterator srv_it = server.begin();
		 srv_it != server.end(); ++srv_it) {
		out << "\n--- Server on port " << srv_it->first << " ---\n";
		const ServerConfig srv = srv_it->second;

		out << "  Host: " << srv.host << '\n';
		out << "  Port: " << srv.port << '\n';
		out << "  Root: " << srv.root << '\n';

		out << "  Index: ";
		for (std::vector<std::string>::const_iterator it = srv.index.begin();
			 it != srv.index.end(); ++it) {
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
			for (std::map<int, std::string>::const_iterator it =
					 srv.error_pages.begin();
				 it != srv.error_pages.end(); ++it) {
				out << "    " << it->first << " -> " << it->second << '\n';
			}
		}

		out << "  Methods: ";
		for (std::vector<std::string>::const_iterator it = srv.methods.begin();
			 it != srv.methods.end(); ++it) {
			if (it != srv.methods.begin()) {
				out << ", ";
			}
			out << *it;
		}
		out << '\n';

		if (srv.redirection.first != 0) {
			out << "  Redirection: " << srv.redirection.first << " -> "
				<< srv.redirection.second << '\n';
		}

		out << "  Large Client Header Buffers: "
			<< srv.large_client_header_buffers << '\n';
		out << "  Client Header Buffer Size: " << srv.client_header_buffer_size
			<< '\n';
		if (!srv.upload_store.empty()) {
			out << "  Upload Store: " << srv.upload_store << '\n';
		}

		if (!srv.location.empty()) {
			out << "\n  === Locations (" << srv.location.size() << ") ===\n";
			for (std::map<std::string, Config>::const_iterator loc_it =
					 srv.location.begin();
				 loc_it != srv.location.end(); ++loc_it) {
				out << "\n  --- Location: " << loc_it->first << " ---\n";
				const Config loc = loc_it->second;

				out << "    Root: " << loc.root << '\n';

				out << "    Index: ";
				for (std::vector<std::string>::const_iterator it =
						 loc.index.begin();
					 it != loc.index.end(); ++it) {
					if (it != loc.index.begin()) {
						out << ", ";
					}
					out << *it;
				}
				out << '\n';

				out << "    Autoindex: " << (loc.autoindex ? "on" : "off")
					<< '\n';
				out << "    Client Max Body Size: " << loc.client_max_body_size
					<< '\n';

				if (!loc.error_pages.empty()) {
					out << "    Error Pages:\n";
					for (std::map<int, std::string>::const_iterator it =
							 loc.error_pages.begin();
						 it != loc.error_pages.end(); ++it) {
						out << "      " << it->first << " -> " << it->second
							<< '\n';
					}
				}

				out << "    Methods: ";
				for (std::vector<std::string>::const_iterator it =
						 loc.methods.begin();
					 it != loc.methods.end(); ++it) {
					if (it != loc.methods.begin()) {
						out << ", ";
					}
					out << *it;
				}
				out << '\n';

				if (loc.redirection.first != 0) {
					out << "    Redirection: " << loc.redirection.first
						<< " -> " << loc.redirection.second << '\n';
				}

				out << "    Large Client Header Buffers: "
					<< loc.large_client_header_buffers << '\n';
				out << "    Client Header Buffer Size: "
					<< loc.client_header_buffer_size << '\n';
				if (!loc.upload_store.empty()) {
					out << "    Upload Store: " << loc.upload_store << '\n';
				}

				if (!loc.cgi_handlers.empty()) {
					out << "    CGI handlers:\n";
					for (std::map<std::string, std::string>::const_iterator it =
							 loc.cgi_handlers.begin();
						 it != loc.cgi_handlers.end(); ++it) {
						out << "      " << it->second << " -> " << it->first
							<< '\n';
					}
				}

				if (loc.redirection.first) {
					out << "    Return: " << loc.redirection.first << " -> "
						<< loc.redirection.second << '\n';
				}
			}
		}
	}

	out << "\n===========================\n";
	return out.str();
}
