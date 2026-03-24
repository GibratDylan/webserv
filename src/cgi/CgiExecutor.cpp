/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Copilot <copilot@openai.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 11:00:00 by Copilot           #+#    #+#             */
/*   Updated: 2026/04/02 11:00:00 by Copilot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cgi/CgiExecutor.hpp"

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <sstream>
#include <stdexcept>

#include "../../include/config/Config.hpp"
#include "../../include/server/exceptions.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"

extern char** environ;

CgiExecutor::CgiExecutor(const std::string& path, const std::string& uri,
						 const std::string& query, const std::string& method,
						 const std::string& body,
						 const std::map<std::string, std::string>& headers,
						 const std::string& app, const Config* config)
	: _pid(-1),
	  _startTime(0),
	  _exitStatus(-1),
	  _state(WRITING),
	  _writeBuffer(body) {
	_fdToCgi[0] = -1;
	_fdToCgi[1] = -1;
	_fdFromCgi[0] = -1;
	_fdFromCgi[1] = -1;

	_env =
		CgiExecutor::createEnv(query, method, body, headers, path, uri, config);

	if (pipe(_fdToCgi) != 0) {
		throw std::runtime_error("Failed to create pipe to CGI");
	}

	if (pipe(_fdFromCgi) != 0) {
		close(_fdToCgi[0]);
		close(_fdToCgi[1]);
		_fdToCgi[0] = -1;
		_fdToCgi[1] = -1;
		throw std::runtime_error("Failed to create pipe from CGI");
	}

	fcntl(_fdToCgi[1], F_SETFL, O_NONBLOCK);
	fcntl(_fdFromCgi[0], F_SETFL, O_NONBLOCK);

	_argv.push_back(app);
	_argv.push_back(path);

	Logger::debug(std::string(" CGI configured script=") + path +
				  " body_bytes=" + toString(_writeBuffer.size()));
}

CgiExecutor::~CgiExecutor() {
	if (_fdToCgi[0] >= 0) {
		close(_fdToCgi[0]);
	}
	if (_fdToCgi[1] >= 0) {
		close(_fdToCgi[1]);
	}
	if (_fdFromCgi[0] >= 0) {
		close(_fdFromCgi[0]);
	}
	if (_fdFromCgi[1] >= 0) {
		close(_fdFromCgi[1]);
	}

	if (_pid > 0) {
		int status = 0;
		pid_t result = waitpid(_pid, &status, WNOHANG);
		if (result == 0) {
			kill(_pid, SIGKILL);
			waitpid(_pid, &status, 0);
		}
	}
}

std::vector<std::string> CgiExecutor::createEnv(
	const std::string& query, const std::string& method,
	const std::string& body, const std::map<std::string, std::string>& headers,
	const std::string& path, const std::string& uri, const Config* config) {
	std::vector<std::string> result;

	for (int i = 0; environ[i] != NULL; ++i) {
		result.push_back(environ[i]);
	}

	result.push_back("GATEWAY_INTERFACE=CGI/1.1");
	result.push_back("SERVER_PROTOCOL=HTTP/1.1");
	result.push_back("REQUEST_METHOD=" + method);
	result.push_back(query.empty() ? "REQUEST_URI=" + uri
								   : "REQUEST_URI=" + uri + "?" + query);
	result.push_back("SCRIPT_FILENAME=" + path);
	result.push_back("SCRIPT_NAME=" + path);
	result.push_back("QUERY_STRING=" + query);
	result.push_back("PATH_INFO=" + uri);
	result.push_back("REDIRECT_STATUS=200");

	std::map<std::string, std::string>::const_iterator ct_it =
		headers.find("Content-Type");
	if (ct_it != headers.end()) {
		result.push_back("CONTENT_TYPE=" + ct_it->second);
	} else {
		result.push_back("CONTENT_TYPE=");
	}

	std::ostringstream size_body;
	size_body << body.size();
	result.push_back("CONTENT_LENGTH=" + size_body.str());

	std::ostringstream port;
	port << config->port;
	result.push_back("SERVER_PORT=" + port.str());
	result.push_back("SERVER_NAME=" + config->host);

	std::map<std::string, std::string>::const_iterator host_it =
		headers.find("Host");
	if (host_it != headers.end()) {
		std::string addr = host_it->second;
		size_t pos = addr.find(':');
		if (pos == std::string::npos) {
			pos = addr.size();
		}
		result.push_back("REMOTE_ADDR=" + addr.substr(0, pos));
	} else {
		result.push_back("REMOTE_ADDR=");
	}

	for (std::map<std::string, std::string>::const_iterator it =
			 headers.begin();
		 it != headers.end(); ++it) {
		std::string key = it->first;
		for (std::string::iterator it_char = key.begin(); it_char != key.end();
			 ++it_char) {
			if (*it_char == '-') {
				*it_char = '_';
			}
			*it_char = static_cast<char>(
				toupper(static_cast<unsigned char>(*it_char)));
		}
		result.push_back("HTTP_" + key + "=" + it->second);
	}

	Logger::debug(std::string(" CGI env built entries=") +
				  toString(result.size()) + " for path=" + path);

	return result;
}

bool CgiExecutor::run() {
	Logger::debug(std::string(" CGI start: ") + _argv[0] + " " + _argv[1]);
	Logger::info(std::string(" CGI launching script ") + _argv[1]);
	_pid = fork();

	if (_pid < 0) {
		Logger::error(" CGI fork failed");
		close(_fdToCgi[0]);
		close(_fdToCgi[1]);
		close(_fdFromCgi[0]);
		close(_fdFromCgi[1]);
		_fdToCgi[0] = -1;
		_fdToCgi[1] = -1;
		_fdFromCgi[0] = -1;
		_fdFromCgi[1] = -1;
		return false;
	}

	if (_pid == 0) {
		close(_fdToCgi[1]);
		if (dup2(_fdToCgi[0], STDIN_FILENO) < 0) {
			throw ExecveException();
		}
		close(_fdToCgi[0]);
		close(_fdFromCgi[0]);
		if (dup2(_fdFromCgi[1], STDOUT_FILENO) < 0) {
			throw ExecveException();
		}
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

		execve(argv_c[0], &argv_c[0], &env_c[0]);
		throw ExecveException();
	}

	close(_fdToCgi[0]);
	_fdToCgi[0] = -1;
	close(_fdFromCgi[1]);
	_fdFromCgi[1] = -1;
	_startTime = std::time(NULL);
	_state = WRITING;
	Logger::debug(std::string(" CGI started with pid ") + toString(_pid));
	return true;
}

void CgiExecutor::onRead() {
	char buffer[65536];
	ssize_t bytes = read(getReadFd(), buffer, sizeof(buffer));

	if (bytes > 0) {
		_readBuffer.append(buffer, static_cast<size_t>(bytes));
	}

	if (bytes == 0) {
		if (!checkProcess()) {
			_startTime = 0;
			_state = DONE;
		}
		return;
	}

	if (bytes < 0) {
		if (!checkProcess()) {
			_startTime = 0;
			_state = DONE;
		}
	}
}

void CgiExecutor::onWrite() {
	if (_state != WRITING) {
		return;
	}

	if (_writeBuffer.empty()) {
		if (_fdToCgi[1] >= 0) {
			close(_fdToCgi[1]);
			_fdToCgi[1] = -1;
		}
		_state = READING;
		return;
	}

	ssize_t sent =
		write(getWriteFd(), _writeBuffer.c_str(), _writeBuffer.size());
	if (sent > 0) {
		_writeBuffer.erase(0, static_cast<size_t>(sent));
	} else if (sent < 0) {
		return;
	}

	if (_writeBuffer.empty()) {
		if (_fdToCgi[1] >= 0) {
			close(_fdToCgi[1]);
			_fdToCgi[1] = -1;
		}
		_state = READING;
	}
}

bool CgiExecutor::checkProcess() {
	if (_pid <= 0) {
		return false;
	}

	int status = 0;
	pid_t result = waitpid(_pid, &status, WNOHANG);
	if (result == 0) {
		return true;
	}
	if (result == _pid) {
		if (WIFEXITED(status)) {
			_exitStatus = WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			_exitStatus = 128 + WTERMSIG(status);
		} else {
			_exitStatus = 1;
		}
		_pid = -1;
		return false;
	}

	Logger::error(" CGI waitpid failed");
	return false;
}

bool CgiExecutor::hasTimedOut() const {
	if (_startTime == 0) {
		return false;
	}
	time_t now = std::time(NULL);
	return (now - _startTime) > 380;
}

bool CgiExecutor::isDone() const {
	return _state == DONE;
}

int CgiExecutor::getReadFd() const {
	return _fdFromCgi[0];
}

int CgiExecutor::getWriteFd() const {
	return _fdToCgi[1];
}

int CgiExecutor::getExitStatus() const {
	return _exitStatus;
}

CgiExecutor::State CgiExecutor::getState() const {
	return _state;
}

const std::string& CgiExecutor::getOutput() const {
	return _readBuffer;
}

bool CgiExecutor::hasOutput() const {
	return !_readBuffer.empty();
}
