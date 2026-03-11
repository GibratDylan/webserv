#include "../../include/Connection.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../../include/FileHandler.h"
#include "../../include/Server.h"
#include "../../include/SessionManager.h"
#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/utility/Cache.h"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utils.h"

const size_t useCache = true;
const size_t kSmallGetRequestMaxBytes = 1024;
const time_t kGetCacheTtlSeconds = 10;
GetResponseCache gCache(kGetCacheTtlSeconds);

Connection::Connection(int fd, const ServerConfig& cfg, SessionManager& sessionMgr)
	: _fd(fd), _state(READING), _request(*this), _lastActivity(std::time(NULL)), config(cfg), sessionManager(sessionMgr), cgi(NULL) {}

Connection::~Connection() {
	close(_fd);
	if (cgi) {
		delete cgi;
	}
}

Connection::State Connection::getState() const {
	return _state;
}

void Connection::setState(Connection::State state) {
	_state = state;
}

bool Connection::isDone() const {
	return _state == DONE;
}

bool Connection::isTimeout(time_t now) const {
	return (now - _lastActivity > 30);
}

void Connection::reset() {
	_readBuffer.clear();
	_writeBuffer.clear();
	_request = HttpRequest(*this);
	_response = HttpResponse();
	_state = READING;
	if (cgi) {
		delete cgi;
		cgi = NULL;
	}
}

void Connection::readFromSocket() {
	char buffer[4096];
	ssize_t bytes;

	while ((bytes = recv(_fd, buffer, sizeof(buffer), 0)) > 0) {
		_readBuffer.append(buffer, bytes);
		_lastActivity = std::time(NULL);
	}

	if (bytes == 0) {
		Logger::debug(std::string(" Connection closed by peer fd=") + toString(_fd) + " buffered_bytes=" + toString(_readBuffer.size()));
		_state = DONE;
		return;
	}

	if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		Logger::error(std::string(" Socket read error fd=") + toString(_fd) + ": " + strerror(errno));
		_state = DONE;
	}
}

void Connection::onWrite() {
	// Logger::debug(std::string(" Writing response fd=") + toString(_fd) + " pending_bytes=" + toString(_writeBuffer.size()));
	if (_state != WRITING) {
		return;
	}

	if (_writeBuffer.empty()) {
		return;
	}

	ssize_t sent = send(_fd, _writeBuffer.c_str(), _writeBuffer.size(), 0);

	if (sent > 0) {
		// Logger::debug(std::string(" Sent bytes fd=") + toString(_fd) + " count=" + toString(sent));
		_writeBuffer.erase(0, sent);
		_lastActivity = std::time(NULL);
	} else if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		Logger::error(std::string(" Socket write error fd=") + toString(_fd) + ": " + strerror(errno));
		_state = DONE;
		return;
	}

	if (_writeBuffer.empty()) {
		if (_request.headers.count("Connection") && _request.headers.at("Connection") == "keep-alive") {
			Logger::debug(std::string(" Keep-alive reset fd=") + toString(_fd));
			reset();
		} else {
			// Logger::debug(std::string(" Connection done fd=") + toString(_fd));
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

void Connection::processRequest() {
	ParseStatus status = _request.parse(_readBuffer);
	if (status == PARSE_INCOMPLETE) {
		return;
	}

	if (status != PARSE_OK) {
		// Logger::info(std::string(" Request parse failed fd=") + toString(_fd) + " status=" + toString(status));
		_response = HttpResponse::makeErrorResponse(status, config);
		_writeBuffer = _response.build();
		_state = WRITING;
		return;
	}

	handleSession();
	_state = PROCESSING;
	const Config& resolvedConfig = config.resolveConfig(_request.path);

	bool cacheableGetRequest = useCache && (_request.method == "GET" && _readBuffer.size() < kSmallGetRequestMaxBytes);
	std::string cacheKey;
	if (cacheableGetRequest) {
		cacheKey = gCache.buildKey(_request, config);
		Logger::debug(std::string(" makeGetResponse Cached ") + toString(Server::countGet++));
		if (gCache.get(cacheKey, _writeBuffer)) {
			_state = WRITING;
			return;
		}
	}
	Logger::info(std::string(" Request resolved fd=") + toString(_fd) + " method=" + _request.method + " path=" + _request.path);

	if (resolvedConfig.redirection.first != 0) {
		_response = HttpResponse::makeRedirectResponse(resolvedConfig.redirection.first, resolvedConfig.redirection.second);
	} else if (resolvedConfig.cgi_handlers.count(getExtension(_request.path))) {
		std::string app = resolvedConfig.cgi_handlers.at(getExtension(_request.path));

		std::string safe_path = FileHandler::normalizePath(_request.path, resolvedConfig.location_path);
		std::string script_path = resolvedConfig.root + safe_path;
		if (!FileSystem::exists(script_path)) {
			// Logger::info(std::string(" CGI script not found: ") + script_path);
			_response = HttpResponse::makeErrorResponse(404, config);
			handleSession();
			_writeBuffer = _response.build();
			_state = WRITING;
			return;
		}

		cgi = new CgiHandler(_request.path, _request.query, _request.method, _request.body, _request.headers, app, const_cast<Config*>(&resolvedConfig));
		if (!cgi->run()) {
			Logger::error(std::string(" CGI launch failed for ") + _request.path);
			delete cgi;
			cgi = NULL;
			_response = HttpResponse::makeErrorResponse(502, config);  // Bad Gateway
			_writeBuffer = _response.build();
			_state = WRITING;
			return;
		}
		_state = INIT_CGI;
		Logger::debug(std::string(" CGI initialized fd=") + toString(_fd));
		return;
	} else if (_request.method == "GET") {
		_response = HttpResponse::makeGetResponse(_request.path, resolvedConfig);
	} else if (_request.method == "POST") {
		_response = HttpResponse::makePostResponse(_request.path, _request.body, resolvedConfig);
	} else if (_request.method == "DELETE") {
		_response = HttpResponse::makeDeleteResponse(_request.path, resolvedConfig);
	} else {
		_response = HttpResponse::makeErrorResponse(405, config);
	}

	handleSession();
	_writeBuffer = _response.build();
	if (cacheableGetRequest && _response.statusCode == 200) {
		gCache.put(cacheKey, _writeBuffer);
	}
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
	// Logger::info(std::string(" CGI finalized fd=") + toString(_fd) + " code=" + toString(_response.statusCode));
	handleSession();
	_writeBuffer = _response.build();
	_state = WRITING;
}

void Connection::handleSession() {
	sessionManager.transferSession(&_request, &_response);
}
