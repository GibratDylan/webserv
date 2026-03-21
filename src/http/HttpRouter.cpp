#include "../../include/http/HttpRouter.hpp"

#include "../../include/config/ServerConfig.hpp"
#include "../../include/server/HttpRequest.hpp"

HttpRouter::HttpRouter(const ServerConfig& serverConfig)
	: _serverConfig(serverConfig) {}

const Config& HttpRouter::resolveLocation(const std::string& path) const {
	return _serverConfig.resolveConfig(path);
}

HttpRouter::HandlerType HttpRouter::determineHandler(
	const HttpRequest& request, const Config& resolvedConfig) const {
	if (resolvedConfig.redirection.first != 0) return HANDLER_REDIRECT;
	if (request.method == "GET") return HANDLER_STATIC_FILE;
	if (request.method == "POST") return HANDLER_UPLOAD;
	if (request.method == "DELETE") return HANDLER_DELETE;
	return HANDLER_STATIC_FILE;
}
