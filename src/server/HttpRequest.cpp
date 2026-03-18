#include "../../include/server/HttpRequest.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "../../include/config/ServerConfig.hpp"
#include "../../include/server/Connection.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"

HttpRequest::HttpRequest(Connection& connection)
	: _complete(false),
	  _headersParsed(false),
	  _headerEnd(0),
	  _contentLength(0),
	  _connection(connection),
	  _resolvedConfig(NULL) {}

HttpRequest::HttpRequest(const HttpRequest& other)
	: _complete(other._complete),
	  _headersParsed(other._headersParsed),
	  _headerEnd(other._headerEnd),
	  _contentLength(other._contentLength),
	  _connection(other._connection),
	  _resolvedConfig(other._resolvedConfig),
	  method(other.method),
	  path(other.path),
	  query(other.query),
	  version(other.version),
	  headers(other.headers),
	  body(other.body) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other) {
		_complete = other._complete;
		_headersParsed = other._headersParsed;
		_headerEnd = other._headerEnd;
		_contentLength = other._contentLength;
		method = other.method;
		path = other.path;
		query = other.query;
		version = other.version;
		headers = other.headers;
		body = other.body;
	}
	return *this;
}

bool HttpRequest::isComplete() const {
	return _complete;
}

std::string HttpRequest::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(name);
	if (it != headers.end()) return it->second;
	return "";
}

ParseStatus HttpRequest::parseHeaders(const std::string& buffer) {
	// Logger::debug(buffer);

	size_t headerEnd = buffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos) return PARSE_INCOMPLETE;

	// If the request-target itself is too large, prefer 414 over 431.
	// (Otherwise, a huge URI can trip the header buffer size check first.)
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
			if (tmpPath.size() >
				(size_t)_connection.config.large_client_header_buffers)
				return URI_TOO_LONG;
		}
	}

	if (headerEnd > (size_t)_connection.config.client_header_buffer_size)
		return HEADER_FIELDS_TOO_LARGE;

	_headerEnd = headerEnd;

	std::string strHeader = buffer.substr(0, headerEnd);
	std::istringstream str(strHeader);
	std::string line;

	if (!std::getline(str, line)) return BAD_REQUEST;

	if (line[line.size() - 1] == '\r') line.erase(line.size() - 1);

	std::istringstream firstLine(line);
	firstLine >> method >> path >> version;
	if (method.empty() || path.empty() || version.empty()) return BAD_REQUEST;

	if (path[path.size() - 1] == '/') path.erase(path.size() - 1);

	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		query = path.substr(queryPos + 1);
		path = path.substr(0, queryPos);
	} else
		query.clear();

	_resolvedConfig = &_connection.config.resolveConfig(path);
	Logger::debug(std::string(" Resolved config path=") + path +
				  " root=" + _resolvedConfig->root);

	if (path.size() > (size_t)_resolvedConfig->large_client_header_buffers)
		return URI_TOO_LONG;
	if (std::find(_resolvedConfig->methods.begin(),
				  _resolvedConfig->methods.end(),
				  method) == _resolvedConfig->methods.end())
		return METHOD_NOT_ALLOWED;

	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		return HTTP_VERSION_NOT_SUPPORTED;

	while (std::getline(str, line)) {
		if (line == "\r") break;

		if (line[line.size() - 1] == '\r') line.erase(line.size() - 1);

		size_t delim = line.find(":");
		if (delim != std::string::npos) {
			std::string key = line.substr(0, delim);
			std::string value = trim(line.substr(delim + 1));
			headers[key] = value;
		}
	}
	return PARSE_OK;
}

ParseStatus HttpRequest::parse(const std::string& buffer) {
	// Logger::debug(std::string(" Parsing request buffer_bytes=") +
	// toString(buffer.size()));

	if (!_headersParsed) {
		Logger::debug(" Parsing headers");
		ParseStatus status = parseHeaders(buffer);
		if (status != PARSE_OK) return status;
		_headersParsed = true;
	}

	if (headers.count("Transfer-Encoding") &&
		headers["Transfer-Encoding"] == "chunked") {
		// Logger::debug(" Parsing chunked body");
		return parseChunked(buffer, _headerEnd, *_resolvedConfig);
	}

	// For requests that may contain a body, HTTP/1.1 requires a framing
	// mechanism. If neither Content-Length nor chunked Transfer-Encoding is
	// provided, reject with 411.
	if ((method == "POST" || method == "PUT") &&
		!headers.count("Content-Length")) {
		return LENGTH_REQUIRED;
	}

	if (headers.count("Content-Length")) {
		std::string lenStr = headers["Content-Length"];
		if (!isNumber(lenStr)) return BAD_REQUEST;

		size_t len =
			static_cast<size_t>(std::strtoul(lenStr.c_str(), NULL, 10));
		if (len > _resolvedConfig->client_max_body_size)
			return PAYLOAD_TOO_LARGE;

		Logger::debug(" Content-Length: " + toString(len));
		std::string strBody = buffer.substr(_headerEnd + 4);

		if (strBody.size() < len) return PARSE_INCOMPLETE;
		body = strBody.substr(0, len);
	}

	_complete = true;
	return PARSE_OK;
}

ParseStatus HttpRequest::parseChunked(const std::string& buffer,
									  size_t headerEnd, const Config& config) {
	size_t pos = headerEnd + 4;	 // "\r\n\r\n"
	std::string decodedBody;

	while (pos < buffer.size()) {
		size_t rnPos = buffer.find("\r\n", pos);
		if (rnPos == std::string::npos) {
			return PARSE_INCOMPLETE;
		}

		std::string sizeStr = buffer.substr(pos, rnPos - pos);

		size_t chunkSize = 0;
		if (std::sscanf(sizeStr.c_str(), "%lx", (unsigned long*)&chunkSize) !=
			1) {
			return BAD_REQUEST;
		}

		pos = rnPos + 2;  // "\r\n"

		if (chunkSize == 0) {
			_complete = true;
			body = decodedBody;
			return PARSE_OK;
		}

		if (pos + chunkSize + 2 > buffer.size()) {
			return PARSE_INCOMPLETE;
		}

		decodedBody += buffer.substr(pos, chunkSize);

		if (decodedBody.size() > (size_t)config.client_max_body_size) {
			return PAYLOAD_TOO_LARGE;
		}

		pos += chunkSize + 2;  // "\r\n"
	}

	return PARSE_INCOMPLETE;
}
