/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Copilot <copilot@openai.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 11:00:00 by Copilot           #+#    #+#             */
/*   Updated: 2026/04/02 11:00:00 by Copilot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cgi/CgiHandler.hpp"

#include <cassert>
#include <cctype>

#include "../../include/config/Config.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"

CgiHandler::CgiHandler(const std::string& path, const std::string& uri,
					   const std::string& query, const std::string& method,
					   const std::string& body,
					   const std::map<std::string, std::string>& headers,
					   const std::string& app, const Config* config)
	: _executor(path, uri, query, method, body, headers, app, config),
	  _state(WRITING),
	  _headersParsed(false),
	  code(500),
	  type("text/html") {}

CgiHandler::~CgiHandler() {}

bool CgiHandler::run() {
	return _executor.run();
}

int CgiHandler::getCgiReadFd() const {
	return _executor.getReadFd();
}

int CgiHandler::getCgiWriteFd() const {
	return _executor.getWriteFd();
}

void CgiHandler::onReadCgi() {
	_executor.onRead();
	if (_executor.getState() == CgiExecutor::DONE) {
		markDone();
	}
}

void CgiHandler::onWriteCgi() {
	if (_state != WRITING) {
		return;
	}

	_executor.onWrite();
	if (_executor.getState() == CgiExecutor::READING) {
		_state = READING;
	}
}

bool CgiHandler::checkProcess() {
	bool running = _executor.checkProcess();
	if (!running && _executor.getState() == CgiExecutor::DONE) {
		markDone();
	}
	return running;
}

void CgiHandler::markDone() {
	if (_state == DONE) {
		return;
	}

	if (_executor.getExitStatus() != 0 && !_executor.hasOutput()) {
		Logger::error(std::string(" CGI script failed with status ") +
					  toString(_executor.getExitStatus()));
		code = 500;
		body = "CGI script failed";
		_state = DONE;
		return;
	}

	if (!_executor.hasOutput()) {
		Logger::error(" CGI script produced no output");
		code = 500;
		body = "CGI script produced no output";
		_state = DONE;
		return;
	}

	parseResponse();
	_state = DONE;
	Logger::info(std::string(" CGI response ready code=") + toString(code) +
				 " body_bytes=" + toString(body.size()));
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
	return _executor.hasTimedOut();
}

void CgiHandler::parseResponse() {
	if (_headersParsed) {
		return;
	}

	CgiResponseParser::ParsedResponse parsed =
		_parser.parse(_executor.getOutput());
	code = parsed.code;
	type = parsed.type;
	body = parsed.body;
	_responseHeaders = parsed.headers;
	_headersParsed = true;

	Logger::info(std::string(" CGI headers parsed code=") + toString(code) +
				 " content_type=" + type +
				 " extra_headers=" + toString(_responseHeaders.size()));
}

HttpResponse CgiHandler::buildResponse() const {
	assert(_state == DONE);

	HttpResponse response(code, HttpResponse::getReason(code));

	response.body = body;
	response.addHeader("Content-Type", type);
	response.addHeader("Content-Length", toString(body.size()));

	for (std::map<std::string, std::string>::const_iterator it =
			 _responseHeaders.begin();
		 it != _responseHeaders.end(); ++it) {
		std::string lowerKey = it->first;
		for (size_t i = 0; i < lowerKey.size(); ++i) {
			lowerKey[i] = static_cast<char>(
				std::tolower(static_cast<unsigned char>(lowerKey[i])));
		}

		if (lowerKey != "status" && lowerKey != "content-type" &&
			lowerKey != "content-length") {
			response.addHeader(it->first, it->second);
		}
	}

	return response;
}
