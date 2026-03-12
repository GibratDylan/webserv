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
class Config;

class HttpRequest {
   private:
	bool _complete;
	bool _headersParsed;
	size_t _headerEnd;
	size_t _contentLength;
	Connection& _connection;
	const Config* _resolvedConfig;
	ParseStatus parseChunked(const std::string& buffer, size_t headerEnd, const Config& config);
	ParseStatus parseHeaders(const std::string& buffer);

   public:
	std::string method;
	std::string path;
	std::string query;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;

	HttpRequest(Connection& connection);
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);

	ParseStatus parse(const std::string& raw);
	bool isComplete() const;
	std::string getHeader(const std::string& name) const;
};
