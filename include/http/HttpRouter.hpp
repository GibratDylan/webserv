#pragma once

#include <string>

class Config;
class ServerConfig;
class HttpRequest;

class HttpRouter {
   public:
	enum HandlerType {
		HANDLER_STATIC_FILE,
		HANDLER_DIRECTORY_LISTING,
		HANDLER_CGI,
		HANDLER_UPLOAD,
		HANDLER_DELETE,
		HANDLER_REDIRECT
	};

	explicit HttpRouter(const ServerConfig& serverConfig);

	const Config& resolveLocation(const std::string& path) const;
	HandlerType determineHandler(const HttpRequest& request,
								 const Config& resolvedConfig) const;

   private:
	const ServerConfig& _serverConfig;
};
