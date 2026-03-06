/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 14:27:53 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/06 17:10:42 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/CgiHandler.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cctype>
#include <iostream>
#include <sstream>

#include "../../include/FileHandler.h"
#include "../../include/config/Config.hpp"

CgiHandler::CgiHandler(const std::string& path, const std::string& query,
					   const std::string& method, const std::string& body,
					   const std::map<std::string, std::string>& headers,
					   Config* config)
	: _method(method), code(500) {
	std::string safe_path =
		FileHandler::normalizePath(path, config->location_path);
	std::string root_path = config->root + safe_path;
	std::cout << "makeGetResponse: " << root_path << '\n';

	_env = CgiHandler::createEnv(query, method, body, headers, path, root_path,
								 config);

	if (pipe(_fdToCgi) != 0) {
		throw std::runtime_error("Failed to create pipe to CGI");
	}
	fcntl(_fdToCgi[0], F_SETFL, O_NONBLOCK);
	fcntl(_fdToCgi[1], F_SETFL, O_NONBLOCK);
	if (pipe(_fdFromCgi) != 0) {
		close(_fdFromCgi[0]);
		close(_fdFromCgi[1]);
		throw std::runtime_error("Failed to create pipe from CGI");
	}
	fcntl(_fdFromCgi[0], F_SETFL, O_NONBLOCK);
	fcntl(_fdFromCgi[1], F_SETFL, O_NONBLOCK);

	_argv.push_back(config->cgi.first);
	_argv.push_back(root_path);
}

CgiHandler::~CgiHandler() {}

std::vector<std::string> CgiHandler::createEnv(
	const std::string& query, const std::string& method,
	const std::string& body, const std::map<std::string, std::string>& headers,
	const std::string& path, const std::string& root_path, Config* config) {
	std::vector<std::string> result;

	for (int i = 0; environ[i] != NULL; ++i) {
		result.push_back(environ[i]);
	}

	result.push_back("GATEWAY_INTERFACE=CGI/1.1");
	result.push_back("SERVER_PROTOCOL=HTTP/1.1");
	result.push_back("REQUEST_METHOD=" + method);
	result.push_back(query.empty() ? "REQUEST_URI=" + path
								   : "REQUEST_URI=" + path + "?" + query);
	result.push_back("SCRIPT_FILENAME=" + root_path);
	result.push_back("SCRIPT_NAME=" + path);
	result.push_back("QUERY_STRING=" + query);

	try {
		result.push_back("CONTENT_TYPE=" + headers.at("Accept"));
	} catch (const std::exception& e) {
		result.push_back("CONTENT_TYPE=");
	}

	std::ostringstream size_body;
	size_body << body.size();
	result.push_back("CONTENT_LENGTH=" + size_body.str());

	std::ostringstream port;
	port << config->port;
	result.push_back("SERVER_PORT=" + port.str());
	result.push_back("SERVER_NAME=" + config->host);

	try {
		std::string addr = headers.at("Host");
		size_t pos = addr.find(':');
		if (pos == std::string::npos) {
			pos = addr.size();
		}
		result.push_back("REMOTE_ADDR=" + addr.substr(0, pos));
	} catch (const std::exception& e) {
		result.push_back("REMOTE_ADDR=");
	}

	for (std::map<std::string, std::string>::const_iterator it =
			 headers.begin();
		 it != headers.end(); it++) {
		std::string key = it->first;
		for (std::string::iterator it_char = key.begin(); it_char != key.end();
			 it_char++) {
			if (*it_char == '-') {
				*it_char = '_';
			}
			*it_char = static_cast<char>(toupper(static_cast<int>(*it_char)));
		}
		result.push_back(key + "=" + it->second);
	}

	return result;
}

void CgiHandler::run() {
	int pid = fork();
	if (!pid) {
		close(_fdToCgi[1]);
		dup2(_fdToCgi[0], STDIN_FILENO);
		close(_fdToCgi[0]);
		close(_fdFromCgi[0]);
		dup2(_fdFromCgi[1], STDOUT_FILENO);
		close(_fdFromCgi[1]);

		std::vector<char*> argv_c;
		for (std::vector<std::string>::iterator it = _argv.begin();
			 it != _argv.end(); ++it) {
			argv_c.push_back(const_cast<char*>(it->c_str()));
		}
		argv_c.push_back(NULL);

		std::vector<char*> env_c;
		for (std::vector<std::string>::iterator it = _env.begin();
			 it != _env.end(); ++it) {
			env_c.push_back(const_cast<char*>(it->c_str()));
		}
		env_c.push_back(NULL);

		execve(argv_c[0], argv_c.data(), env_c.data());
	}
}
