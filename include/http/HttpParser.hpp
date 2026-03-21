#pragma once

#include <cstddef>
#include <string>

#include "../server/HttpRequest.hpp"
#include "HttpRequestValidator.hpp"

class Config;
class ServerConfig;

class HttpParser {
   public:
	explicit HttpParser(const ServerConfig& serverConfig);

	ParseStatus parse(const std::string& buffer, HttpRequest& request);
	void reset();
	const Config* getResolvedConfig() const;

   private:
	ParseStatus parseHeaders(const std::string& buffer, HttpRequest& request);
	ParseStatus parseChunkedBody(const std::string& buffer, size_t headerEnd,
								 HttpRequest& request,
								 const Config& resolvedConfig);
	ParseStatus parseContentLengthBody(const std::string& buffer,
									   size_t headerEnd, HttpRequest& request,
									   const Config& resolvedConfig);

   private:
	const ServerConfig& _serverConfig;
	HttpRequestValidator _validator;

	bool _headersParsed;
	size_t _headerEnd;
	const Config* _resolvedConfig;
};
