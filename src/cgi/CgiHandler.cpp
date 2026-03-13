/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 14:27:53 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/11 20:22:06 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cgi/CgiHandler.hpp"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "../../include/FileHandler.h"
#include "../../include/HttpResponse.h"
#include "../../include/config/Config.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utils.h"

CgiHandler::CgiHandler(const std::string& path, const std::string& query, const std::string& method, const std::string& body,
					   const std::map<std::string, std::string>& headers, const std::string& app, Config* config)
	: _method(method), _pid(-1), _startTime(0), _exitStatus(-1), _state(WRITING), _writeBuffer(body), _headersParsed(false), code(500), type("text/html") {
	std::string safe_path = FileHandler::normalizePath(path, config->location_path);
	std::string root_path = config->root + safe_path;

	_env = CgiHandler::createEnv(query, method, body, headers, path, root_path, config);

	if (pipe(_fdToCgi) != 0) {
		throw std::runtime_error("Failed to create pipe to CGI");
	}

	fcntl(_fdToCgi[0], F_SETFL, O_NONBLOCK);
	fcntl(_fdToCgi[1], F_SETFL, O_NONBLOCK);

	if (pipe(_fdFromCgi) != 0) {
		close(_fdToCgi[0]);
		close(_fdToCgi[1]);
		throw std::runtime_error("Failed to create pipe from CGI");
	}

	fcntl(_fdFromCgi[0], F_SETFL, O_NONBLOCK);
	fcntl(_fdFromCgi[1], F_SETFL, O_NONBLOCK);

	_argv.push_back(app);
	_argv.push_back(root_path);

	Logger::debug(std::string(" CGI configured method=") + _method + " script=" + root_path + " body_bytes=" + toString(_writeBuffer.size()));
}

CgiHandler::~CgiHandler() {
	close(_fdToCgi[1]);
	close(_fdFromCgi[0]);

	if (_pid > 0) {
		int status = 0;
		waitpid(_pid, &status, WNOHANG);
		if (waitpid(_pid, &status, WNOHANG) == 0) {
			kill(_pid, SIGKILL);
			waitpid(_pid, &status, 0);
		}
	}
}

std::vector<std::string> CgiHandler::createEnv(const std::string& query, const std::string& method, const std::string& body,
											   const std::map<std::string, std::string>& headers, const std::string& path, const std::string& root_path,
											   Config* config) {
	std::vector<std::string> result;

	for (int i = 0; environ[i] != NULL; ++i) {
		result.push_back(environ[i]);
	}

	result.push_back("GATEWAY_INTERFACE=CGI/1.1");
	result.push_back("SERVER_PROTOCOL=HTTP/1.1");
	result.push_back("REQUEST_METHOD=" + method);
	result.push_back(query.empty() ? "REQUEST_URI=" + path : "REQUEST_URI=" + path + "?" + query);
	result.push_back("SCRIPT_FILENAME=" + root_path);
	result.push_back("SCRIPT_NAME=" + path);
	result.push_back("QUERY_STRING=" + query);
	result.push_back("PATH_INFO=" + path);

	try {
		result.push_back("CONTENT_TYPE=" + headers.at("Content-Type"));
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

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++) {
		std::string key = it->first;
		for (std::string::iterator it_char = key.begin(); it_char != key.end(); it_char++) {
			if (*it_char == '-') {
				*it_char = '_';
			}
			*it_char = static_cast<char>(toupper(static_cast<int>(*it_char)));
		}
		result.push_back("HTTP_" + key + "=" + it->second);
	}

	Logger::debug(std::string(" CGI env built entries=") + toString(result.size()) + " for path=" + path);

	return result;
}

bool CgiHandler::run() {
	Logger::debug(std::string(" CGI start: ") + _argv[0] + " " + _argv[1]);
	Logger::info(std::string(" CGI launching script ") + _argv[1]);
	_pid = fork();

	if (_pid < 0) {
		Logger::error(" CGI fork failed");
		close(_fdToCgi[1]);
		close(_fdToCgi[0]);
		close(_fdFromCgi[1]);
		close(_fdFromCgi[0]);
		return false;
	}

	if (_pid == 0) {
		close(_fdToCgi[1]);
		if (dup2(_fdToCgi[0], STDIN_FILENO) < 0) {
			return false;
		}
		close(_fdToCgi[0]);
		close(_fdFromCgi[0]);
		if (dup2(_fdFromCgi[1], STDOUT_FILENO) < 0) {
			return false;
		}
		close(_fdFromCgi[1]);

		std::vector<char*> argv_c;
		for (std::vector<std::string>::iterator it = _argv.begin(); it != _argv.end(); ++it) {
			argv_c.push_back(const_cast<char*>(it->c_str()));
		}
		argv_c.push_back(NULL);

		std::vector<char*> env_c;
		for (std::vector<std::string>::iterator it = _env.begin(); it != _env.end(); ++it) {
			env_c.push_back(const_cast<char*>(it->c_str()));
		}
		env_c.push_back(NULL);

		execve(argv_c[0], argv_c.data(), env_c.data());
		Logger::error(std::string(" CGI execve failed for ") + argv_c[0]);
		std::exit(127);
	} else if (_pid > 0) {
		close(_fdToCgi[0]);
		close(_fdFromCgi[1]);
		Logger::debug(std::string(" CGI started with pid ") + toString(_pid));
		_startTime = std::time(NULL);
	}

	_state = WRITING;
	return true;
}

int CgiHandler::getCgiReadFd() const {
	return _fdFromCgi[0];
}

int CgiHandler::getCgiWriteFd() const {
	return _fdToCgi[1];
}

void CgiHandler::onReadCgi() {
	// if (_state != READING) {
	// 	return;
	// }

	char buffer[94096];
	ssize_t bytes = read(getCgiReadFd(), buffer, sizeof(buffer));

	if (bytes > 0) {
		_readBuffer.append(buffer, bytes);
		Logger::debug(std::string(" CGI read bytes=") + toString(bytes));
	}

	if (bytes == 0) {
		checkProcess();
		if (_exitStatus == 0 || _readBuffer.size() > 0) {
			parseResponse();
			Logger::info(std::string(" CGI response ready code=") + toString(code) + " body_bytes=" + toString(body.size()));
		} else {
			Logger::error(" CGI script produced no output");
			code = 500;
			body = "CGI script produced no output";
		}
		_startTime = 0;
		_state = DONE;
		return;
	}

	if (bytes < 0) {
		if (!checkProcess()) {
			if (_exitStatus != 0 && _readBuffer.size() == 0) {
				Logger::error(std::string(" CGI script failed with status ") + toString(_exitStatus));
				code = 500;
				body = "CGI script failed";
			} else if (_readBuffer.size() > 0) {
				parseResponse();
			} else {
				Logger::error(" CGI script produced no output");
				code = 500;
				body = "CGI script produced no output";
			}
			_startTime = 0;
			_state = DONE;
		}
	}
}

void CgiHandler::onWriteCgi() {
	if (_state != WRITING) {
		return;
	}

	if (_writeBuffer.empty()) {
		close(_fdToCgi[1]);
		Logger::debug(" CGI stdin closed (no request body)");
		_state = READING;
		return;
	}

	ssize_t sent = write(getCgiWriteFd(), _writeBuffer.c_str(), _writeBuffer.size());

	if (sent > 0) {
		_writeBuffer.erase(0, sent);
	} else if (sent < 0) {
		// Non-blocking pipe may be temporarily full; poll will trigger POLLOUT again.
		return;
	}

	if (_writeBuffer.empty()) {
		close(_fdToCgi[1]);
		Logger::debug(" CGI stdin closed (request body sent)");
		_state = READING;
	}
}

bool CgiHandler::checkProcess() {
	if (_pid <= 0) {
		return false;
	}

	int status = 0;
	pid_t result = waitpid(_pid, &status, WNOHANG);

	if (result == 0) {
		return true;
	} else if (result == _pid) {
		if (WIFEXITED(status)) {
			_exitStatus = WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			_exitStatus = 128 + WTERMSIG(status);
		} else {
			_exitStatus = 1;
		}
		if (_exitStatus != 0) {
			Logger::warning(std::string(" CGI process exited with status ") + toString(_exitStatus));
		} else {
			Logger::debug(" CGI process exited successfully");
		}
		_pid = -1;
		return false;
	} else if (result < 0) {
		Logger::error(" CGI waitpid failed");
		return false;
	}

	return false;
}

bool CgiHandler::isDone() const {
	return _state == DONE;
}

CgiHandler::State CgiHandler::getState() const {
	return _state;
}

int CgiHandler::getCode() const {
	return code;
}

bool CgiHandler::hasTimedOut() const {
	if (_startTime == 0) {
		return false;
	}
	time_t now = std::time(NULL);
	return (now - _startTime) > 60;
}

void CgiHandler::parseResponse() {
	if (_headersParsed) {
		return;
	}

	// Logger::debug(std::string(" CGI parsing response buffer_bytes=") + toString(_readBuffer.size()));

	if (_readBuffer.empty()) {
		code = 500;
		body = "CGI script produced no output";
		_headersParsed = true;
		return;
	}

	size_t headerEnd = _readBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		headerEnd = _readBuffer.find("\n\n");
		if (headerEnd != std::string::npos) {
			body = _readBuffer.substr(headerEnd + 2);
		} else {
			body = _readBuffer;
			code = 200;
			_headersParsed = true;
			return;
		}
	} else {
		body = _readBuffer.substr(headerEnd + 4);
	}

	std::string headerSection = _readBuffer.substr(0, headerEnd);
	std::istringstream stream(headerSection);
	std::string line;

	code = 200;

	while (std::getline(stream, line)) {
		if (line.empty() || line == "\r") {
			break;
		}

		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) {
			continue;
		}

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos) {
			value = value.substr(start);
		}
		size_t end = value.find_last_not_of(" \t");
		if (end != std::string::npos) {
			value = value.substr(0, end + 1);
		}

		std::string lowerKey = key;
		for (size_t i = 0; i < lowerKey.size(); ++i) {
			lowerKey[i] = std::tolower(lowerKey[i]);
		}

		if (lowerKey == "status") {
			std::istringstream codeStream(value);
			codeStream >> code;
		} else if (lowerKey == "content-type") {
			type = value;
		}

		_responseHeaders[key] = value;
	}

	_headersParsed = true;
	Logger::info(std::string(" CGI headers parsed code=") + toString(code) + " content_type=" + type + " extra_headers=" + toString(_responseHeaders.size()));
}

HttpResponse CgiHandler::buildResponse() const {
	HttpResponse response(code, HttpResponse::getReason(code));

	response.body = body;
	response.addHeader("Content-Type", type);
	response.addHeader("Content-Length", toString(body.size()));

	for (std::map<std::string, std::string>::const_iterator it = _responseHeaders.begin(); it != _responseHeaders.end(); ++it) {
		std::string lowerKey = it->first;
		for (size_t i = 0; i < lowerKey.size(); ++i) {
			lowerKey[i] = std::tolower(lowerKey[i]);
		}

		if (lowerKey != "status" && lowerKey != "content-type" && lowerKey != "content-length") {
			response.addHeader(it->first, it->second);
		}
	}

	return response;
}
