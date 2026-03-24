/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/02 13:25:13 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/Connection.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/server/Server.hpp"
#include "../../include/server/SessionManager.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Cache.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utility/PathUtils.hpp"

// const bool useCache = false;
// const size_t kSmallGetRequestMaxBytes = 1024;
const time_t kGetCacheTtlSeconds = 10;
GetResponseCache gCache(kGetCacheTtlSeconds);

Connection::Connection(int fd, const ServerConfig& cfg,
					   SessionManager& sessionManager)
	: _fd(fd),
	  _state(READING),
	  _request(),
	  _parser(cfg),
	  _router(cfg),
	  _responseBuilder(),
	  _lastActivity(std::time(NULL)),
	  config(cfg),
	  sessionManager(sessionManager),
	  cgi(NULL) {}

Connection::~Connection() {
	close(_fd);
	delete cgi;
}

Connection::State Connection::getState() const {
	return _state;
}

int Connection::getFd() const {
	return _fd;
}

void Connection::setState(Connection::State state) {
	_state = state;
}

bool Connection::isDone() const {
	return _state == DONE;
}

bool Connection::isTimeout(time_t now) const {
	return (now - _lastActivity > 380);
}

void Connection::reset() {
	_readBuffer.clear();
	_writeBuffer.clear();
	_request = HttpRequest();
	_response = HttpResponse();
	_parser.reset();
	_state = READING;
	delete cgi;
	cgi = NULL;
}

void Connection::readFromSocket() {
	char buffer[94096];
	ssize_t bytes = recv(_fd, buffer, sizeof(buffer), MSG_DONTWAIT);

	if (bytes > 0) {
		_readBuffer.append(buffer, bytes);
		_lastActivity = std::time(NULL);
	}

	if (bytes == 0) {
		Logger::debug(std::string(" Connection closed by peer fd=") +
					  toString(_fd) +
					  " buffered_bytes=" + toString(_readBuffer.size()));
		_state = DONE;
		return;
	}
}

void Connection::onWrite() {
	Logger::debug(std::string(" Writing response fd=") + toString(_fd) +
				  " pending_bytes=" + toString(_writeBuffer.size()));
	if (_state != WRITING) {
		return;
	}

	if (_writeBuffer.empty()) {
		return;
	}

	ssize_t sent = send(_fd, _writeBuffer.c_str(), _writeBuffer.size(),
						MSG_DONTWAIT | MSG_NOSIGNAL);

	if (sent > 0) {
		Logger::debug(std::string(" Sent bytes fd=") + toString(_fd) +
					  " count=" + toString(sent));
		_writeBuffer.erase(0, sent);
		_lastActivity = std::time(NULL);
	}

	if (_writeBuffer.empty()) {
		if (_request.headers.count("Connection") &&
			_request.headers.at("Connection") == "keep-alive") {
			Logger::debug(std::string(" Keep-alive reset fd=") + toString(_fd));
			reset();
		} else {
			Logger::debug(std::string(" Connection done fd=") + toString(_fd));
			_state = DONE;
		}
	}
}

void Connection::onRead() {
	bool shouldProcessRequest = (_state == READING);

	if (_state != READING && _state != PROCESSING_CGI) {
		return;
	}

	readFromSocket();

	if (_state == DONE) {
		return;
	}

	if (shouldProcessRequest) {
		processRequest();
	}
}

void Connection::resolveCgiTarget(const Config& resolvedConfig,
								  std::string& cgiRequestPath,
								  std::string& cgiExtension) {
	cgiRequestPath = _request.path;
	cgiExtension = PathUtils::getExtension(_request.path);

	if (!cgiExtension.empty()) return;

	std::string safePath = PathUtils::normalizeForLocation(
		_request.path, resolvedConfig.location_path);

	std::string rootPath = addPath(resolvedConfig.root, safePath);
	if (!FileSystem::isDirectory(rootPath)) return;

	std::string indexFilePath;
	std::string indexName;
	if (!FileSystem::findIndexFile(rootPath, resolvedConfig.index,
								   indexFilePath, indexName))
		return;

	std::string indexExt = PathUtils::getExtension(indexFilePath);
	if (!resolvedConfig.cgi_handlers.count(indexExt)) return;

	cgiExtension = indexExt;
	if (!cgiRequestPath.empty() &&
		cgiRequestPath[cgiRequestPath.size() - 1] == '/')
		cgiRequestPath += indexName;
	else
		cgiRequestPath += "/" + indexName;
}

bool Connection::tryStartCgi(const Config& resolvedConfig,
							 const std::string& cgiRequestPath,
							 const std::string& cgiExtension) {
	if (!resolvedConfig.cgi_handlers.count(cgiExtension)) {
		return false;
	}

	std::string app = resolvedConfig.cgi_handlers.at(cgiExtension);
	std::string safePath = PathUtils::normalizeForLocation(
		cgiRequestPath, resolvedConfig.location_path);
	std::string scriptPath = addPath(resolvedConfig.root, safePath);

	if (!FileSystem::exists(scriptPath)) {
		Logger::info(std::string(" CGI script not found: ") + scriptPath);
		_response = HttpResponse::makeErrorResponse(404, config);
		handleSession();
		_writeBuffer = _response.build();
		_state = WRITING;
		return true;
	}
	try {
		cgi = new CgiHandler(scriptPath, cgiRequestPath, _request.query,
							 _request.method, _request.body, _request.headers,
							 app, &resolvedConfig);
	} catch (const std::exception& e) {
		Logger::error(std::string(" CGI creation failed for ") +
					  cgiRequestPath);
		delete cgi;
		cgi = NULL;
		_response = HttpResponse::makeErrorResponse(500, config);
		_writeBuffer = _response.build();
		_state = WRITING;
		return true;
	}
	if (!cgi->run()) {
		Logger::error(std::string(" CGI launch failed for ") + cgiRequestPath);
		delete cgi;
		cgi = NULL;
		_response = HttpResponse::makeErrorResponse(502, config);
		_writeBuffer = _response.build();
		_state = WRITING;
		return true;
	}

	_state = INIT_CGI;
	Logger::debug(std::string(" CGI initialized fd=") + toString(_fd));
	return true;
}

void Connection::processRequest() {
	ParseStatus status = _parser.parse(_readBuffer, _request);
	if (status == PARSE_INCOMPLETE) {
		return;
	}

	if (status != PARSE_OK) {
		Logger::info(std::string(" Request parse failed fd=") + toString(_fd) +
					 " status=" + toString(status));
		_response = _responseBuilder.makeError(status, config);
		_writeBuffer = _response.build();
		_state = WRITING;
		return;
	}

	handleSession();
	_state = PROCESSING;
	const Config& resolvedConfig = _router.resolveLocation(_request.path);
	std::string cgiRequestPath;
	std::string cgiExtension;
	resolveCgiTarget(resolvedConfig, cgiRequestPath, cgiExtension);

	// bool cacheableGetRequest = useCache && (_request.method == "GET" &&
	// _readBuffer.size() < kSmallGetRequestMaxBytes); std::string cacheKey; if
	// (cacheableGetRequest) { 	cacheKey = gCache.buildKey(_request, config);
	// 	Logger::debug(std::string(" makeGetResponse Cached ") +
	// toString(Server::countGet++)); 	if (gCache.get(cacheKey, _writeBuffer))
	// { 		_state = WRITING; 		return;
	// 	}
	// }
	Logger::info(std::string(" Request resolved fd=") + toString(_fd) +
				 " method=" + _request.method + " path=" + _request.path);

	HttpRouter::HandlerType handlerType =
		_router.determineHandler(_request, resolvedConfig);

	if (handlerType == HttpRouter::HANDLER_REDIRECT) {
		_response =
			_responseBuilder.makeRedirect(resolvedConfig.redirection.first,
										  resolvedConfig.redirection.second);
	} else if (tryStartCgi(resolvedConfig, cgiRequestPath, cgiExtension)) {
		return;
	} else if (handlerType == HttpRouter::HANDLER_STATIC_FILE &&
			   _request.method == "GET") {
		_response =
			HttpResponse::makeGetResponse(_request.path, resolvedConfig);
	} else if (handlerType == HttpRouter::HANDLER_UPLOAD &&
			   _request.method == "POST") {
		_response = HttpResponse::makePostResponse(_request.path, _request.body,
												   resolvedConfig);
	} else if (handlerType == HttpRouter::HANDLER_DELETE &&
			   _request.method == "DELETE") {
		_response =
			HttpResponse::makeDeleteResponse(_request.path, resolvedConfig);
	} else {
		_response = _responseBuilder.makeError(405, config);
	}

	handleSession();
	_writeBuffer = _response.build();
	// if (cacheableGetRequest && _response.statusCode == 200) {
	// 	gCache.put(cacheKey, _writeBuffer);
	// }
	_state = WRITING;
}

void Connection::finalizeCgi() {
	if (!cgi) {
		return;
	}

	if (cgi->hasTimedOut()) {
		_response = HttpResponse::makeErrorResponse(504, config);
	} else if (cgi->getCode() > 400 && cgi->getCode() < 600) {
		_response = HttpResponse::makeErrorResponse(cgi->getCode(), config);
	} else {
		_response = cgi->buildResponse();
	}
	Logger::info(std::string(" CGI finalized fd=") + toString(_fd) +
				 " code=" + toString(_response.statusCode));
	handleSession();
	_writeBuffer = _response.build();
	_state = WRITING;
}

void Connection::handleSession() {
	sessionManager.transferSession(&_request, &_response);
}
