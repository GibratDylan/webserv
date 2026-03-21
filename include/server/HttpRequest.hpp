#pragma once

#include <map>
#include <string>

enum ParseStatus {
	PARSE_OK = 0,
	PARSE_INCOMPLETE = -1,

	BAD_REQUEST = 400,
	METHOD_NOT_ALLOWED = 405,
	LENGTH_REQUIRED = 411,
	PAYLOAD_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	CONFLICT = 409,
	HEADER_FIELDS_TOO_LARGE = 431,
	HTTP_VERSION_NOT_SUPPORTED = 505
};

class Connection;

class HttpRequest {
   public:
	std::string method;
	std::string path;
	std::string query;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;

	HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	std::string getHeader(const std::string& name) const;
};
