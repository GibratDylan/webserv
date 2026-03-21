#pragma once

#include <map>
#include <string>

#include "HttpResponse.hpp"

class Config;

class HttpResponseBuilder {
   public:
	HttpResponseBuilder();

	HttpResponseBuilder& setStatus(int code);
	HttpResponseBuilder& setHeader(const std::string& key,
								   const std::string& value);
	HttpResponseBuilder& setBody(const std::string& body);
	HttpResponseBuilder& setContentType(const std::string& type);

	HttpResponse build() const;

	HttpResponse makeError(int code, const Config& config) const;
	HttpResponse makeRedirect(int code, const std::string& location) const;

   private:
	int _statusCode;
	std::string _reason;
	std::map<std::string, std::string> _headers;
	std::string _body;
};
