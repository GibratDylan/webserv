/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/HttpParser.hpp"

#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "../../include/config/ServerConfig.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"

HttpParser::HttpParser(const ServerConfig& serverConfig)
	: _serverConfig(serverConfig),
	  _headersParsed(false),
	  _headerEnd(0),
	  _resolvedConfig(NULL) {}

const Config* HttpParser::getResolvedConfig() const {
	return _resolvedConfig;
}

void HttpParser::reset() {
	_headersParsed = false;
	_headerEnd = 0;
	_resolvedConfig = NULL;
}

ParseStatus HttpParser::parse(const std::string& buffer, HttpRequest& request) {
	if (!_headersParsed) {
		ParseStatus status = parseHeaders(buffer, request);
		if (status != PARSE_OK) return status;
		_headersParsed = true;
	}

	if (!_resolvedConfig) return BAD_REQUEST;

	ParseStatus framingStatus =
		_validator.validateBodyFraming(request.method, request.headers);
	if (framingStatus != PARSE_OK) return framingStatus;

	if (request.headers.count("Transfer-Encoding") &&
		request.headers.find("Transfer-Encoding")->second == "chunked") {
		return parseChunkedBody(buffer, _headerEnd, request, *_resolvedConfig);
	}

	return parseContentLengthBody(buffer, _headerEnd, request,
								  *_resolvedConfig);
}

ParseStatus HttpParser::parseHeaders(const std::string& buffer,
									 HttpRequest& request) {
	size_t headerEnd = buffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos) return PARSE_INCOMPLETE;

	// Prefer 414 if request-target too large.
	size_t firstLineEnd = buffer.find("\r\n");
	if (firstLineEnd == std::string::npos) return PARSE_INCOMPLETE;
	{
		std::string firstLineRaw = buffer.substr(0, firstLineEnd);
		std::istringstream firstLine(firstLineRaw);
		std::string tmpMethod;
		std::string tmpPath;
		std::string tmpVersion;
		firstLine >> tmpMethod >> tmpPath >> tmpVersion;
		if (!tmpPath.empty()) {
			if (tmpPath[tmpPath.size() - 1] == '/')
				tmpPath.erase(tmpPath.size() - 1);
			ParseStatus uriStatus = _validator.validateUriLength(
				tmpPath,
				static_cast<size_t>(_serverConfig.large_client_header_buffers));
			if (uriStatus != PARSE_OK) return uriStatus;
		}
	}

	ParseStatus headerSizeStatus = _validator.validateHeaderSize(
		headerEnd,
		static_cast<size_t>(_serverConfig.client_header_buffer_size));
	if (headerSizeStatus != PARSE_OK) return headerSizeStatus;

	_headerEnd = headerEnd;

	std::string strHeader = buffer.substr(0, headerEnd);
	std::istringstream stream(strHeader);
	std::string line;

	if (!std::getline(stream, line)) return BAD_REQUEST;
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	std::istringstream firstLine(line);
	firstLine >> request.method >> request.path >> request.version;
	if (request.method.empty() || request.path.empty() ||
		request.version.empty())
		return BAD_REQUEST;

	if (!request.path.empty() && request.path[request.path.size() - 1] == '/')
		request.path.erase(request.path.size() - 1);

	size_t queryPos = request.path.find('?');
	if (queryPos != std::string::npos) {
		request.query = request.path.substr(queryPos + 1);
		request.path = request.path.substr(0, queryPos);
	} else {
		request.query.clear();
	}

	_resolvedConfig = &_serverConfig.resolveConfig(request.path);
	Logger::debug(std::string(" Resolved config path=") + request.path +
				  " root=" + _resolvedConfig->root);

	ParseStatus uriStatus = _validator.validateUriLength(
		request.path,
		static_cast<size_t>(_resolvedConfig->large_client_header_buffers));
	if (uriStatus != PARSE_OK) return uriStatus;

	ParseStatus methodStatus =
		_validator.validateMethod(request.method, *_resolvedConfig);
	if (methodStatus != PARSE_OK) return methodStatus;

	ParseStatus versionStatus = _validator.validateVersion(request.version);
	if (versionStatus != PARSE_OK) return versionStatus;

	request.headers.clear();
	while (std::getline(stream, line)) {
		if (line == "\r") break;
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		size_t delim = line.find(':');
		if (delim != std::string::npos) {
			std::string key = line.substr(0, delim);
			std::string value = trim(line.substr(delim + 1));
			request.headers[key] = value;
		}
	}

	return PARSE_OK;
}

ParseStatus HttpParser::parseContentLengthBody(const std::string& buffer,
											   size_t headerEnd,
											   HttpRequest& request,
											   const Config& resolvedConfig) {
	size_t contentLength = 0;
	ParseStatus lenStatus = _validator.validateContentLength(
		request.headers, resolvedConfig.client_max_body_size, contentLength);
	if (lenStatus != PARSE_OK) return lenStatus;

	if (request.headers.count("Content-Length")) {
		std::string strBody = buffer.substr(headerEnd + 4);
		if (strBody.size() < contentLength) return PARSE_INCOMPLETE;
		request.body = strBody.substr(0, contentLength);
	} else {
		request.body.clear();
	}

	return PARSE_OK;
}

ParseStatus HttpParser::parseChunkedBody(const std::string& buffer,
										 size_t headerEnd, HttpRequest& request,
										 const Config& resolvedConfig) {
	size_t pos = headerEnd + 4;
	std::string decodedBody;

	while (pos < buffer.size()) {
		size_t rnPos = buffer.find("\r\n", pos);
		if (rnPos == std::string::npos) return PARSE_INCOMPLETE;

		std::string sizeStr = buffer.substr(pos, rnPos - pos);
		size_t chunkSize = 0;
		char* endPtr = NULL;
		const unsigned long ul = std::strtoul(sizeStr.c_str(), &endPtr, 16);
		if (!endPtr || *endPtr != '\0') {
			return BAD_REQUEST;
		}
		chunkSize = ul;

		pos = rnPos + 2;
		if (chunkSize == 0) {
			request.body = decodedBody;
			return PARSE_OK;
		}

		if (pos + chunkSize + 2 > buffer.size()) return PARSE_INCOMPLETE;

		decodedBody += buffer.substr(pos, chunkSize);
		if (decodedBody.size() > (size_t)resolvedConfig.client_max_body_size)
			return PAYLOAD_TOO_LARGE;

		pos += chunkSize + 2;
	}

	return PARSE_INCOMPLETE;
}
