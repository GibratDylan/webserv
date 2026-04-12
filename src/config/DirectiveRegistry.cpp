/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveRegistry.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:11 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/DirectiveRegistry.hpp"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

#include "../../include/config/Config.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/server/utils.hpp"

namespace {

static int parseInt(const std::string& s) {
	char* endPtr = NULL;
	long v = std::strtol(s.c_str(), &endPtr, 10);
	if (!endPtr || *endPtr != '\0') {
		throw std::runtime_error("Error: Invalid integer value");
	}
	return static_cast<int>(v);
}

class RootDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1 || args[0].empty() || args[0].at(0) != '.') {
			throw std::runtime_error(
				"Error: 'root' directive requires exactly one argument (start "
				"with '.' )");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		config.root = args[0];
		config.root_explicitly_set = true;
	}
};

class IndexDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& /*args*/) const {}

	void apply(Config& config, const std::vector<std::string>& args) const {
		config.index = args;
	}
};

class ErrorPageDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() < 2 || args.back().empty() ||
			args.back().at(0) != '.') {
			throw std::runtime_error(
				"Error: 'error_page' directive requires at least code and path "
				"(start with '.') arguments");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		const std::string& path = args.back();
		for (size_t i = 0; i + 1 < args.size(); ++i) {
			const int code = parseInt(args[i]);
			if (!HttpResponse::isClientError(code) &&
				!HttpResponse::isServerError(code)) {
				throw std::runtime_error(
					"Error: Invalid error code in 'error_page' directive");
			}
			config.error_pages[code] = path;
		}
	}
};

class AutoIndexDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1 || (args[0] != "off" && args[0] != "on")) {
			throw std::runtime_error(
				"Error: 'autoindex' directive requires exactly one argument "
				"('on' or 'off')");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		config.autoindex = (args[0] == "on");
	}
};

class ClientMaxBodySizeDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1) {
			throw std::runtime_error(
				"Error: 'client_max_body_size' directive requires exactly one "
				"argument");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		try {
			config.client_max_body_size = conversionBytesParsing(args[0]);
		} catch (...) {
			throw std::runtime_error(
				"Error: Invalid value for 'client_max_body_size' directive");
		}
	}
};

class LargeClientHeaderBuffersDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1) {
			throw std::runtime_error(
				"Error: 'large_client_header_buffers' directive requires "
				"exactly one argument");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		try {
			config.large_client_header_buffers =
				conversionBytesParsing(args[0]);
		} catch (...) {
			throw std::runtime_error(
				"Error: Invalid value for 'large_client_header_buffers' "
				"directive");
		}
	}
};

class ClientHeaderBufferSizeDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1) {
			throw std::runtime_error(
				"Error: 'client_header_buffer_size' directive requires exactly "
				"one argument");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		try {
			config.client_header_buffer_size = conversionBytesParsing(args[0]);
		} catch (...) {
			throw std::runtime_error(
				"Error: Invalid value for 'client_header_buffer_size' "
				"directive");
		}
	}
};

class UploadStoreDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1) {
			throw std::runtime_error(
				"Error: 'upload_store' directive requires exactly one "
				"argument");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		config.upload_store = args[0];
	}
};

class AllowMethodsDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.empty() || args.size() > 3) {
			throw std::runtime_error(
				"Error: 'allow_methods' directive requires at least one "
				"argument and maximum three");
		}
		static const char* valid[] = {"GET", "POST", "DELETE"};
		for (size_t i = 0; i < args.size(); ++i) {
			bool ok = false;
			for (size_t j = 0; j < sizeof(valid) / sizeof(valid[0]); ++j) {
				if (args[i] == valid[j]) {
					ok = true;
					break;
				}
			}
			if (!ok) {
				throw std::runtime_error(
					"Error: 'allow_methods' directive requires GET, POST or "
					"DELETE");
			}
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		config.methods = args;
	}
};

class ReturnDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.empty() || args.size() > 3) {
			throw std::runtime_error(
				"Error: 'return' directive requires exactly 1 or 2 argument");
		}
		const int code = parseInt(args[0]);
		if (!HttpResponse::isValid(code)) {
			throw std::runtime_error(
				"Error: Invalid error code in 'return' directive");
		}
		static_cast<void>(code);
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		const int code = parseInt(args[0]);
		config.redirection.first = code;
		if (args.size() == 2) {
			config.redirection.second = args[1];
		}
	}
};

class CgiDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1 && args.size() != 2) {
			throw std::runtime_error(
				"Error: 'cgi' directive requires one or two arguments "
				"(interpreter [extension])");
		}
		if (args.size() == 2) {
			if (args[1].empty() || args[1].at(0) != '.') {
				throw std::runtime_error(
					"Error: cgi extension must start with .");
			}
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);

		const std::string& app = args[0];
		std::string extension;

		if (args.size() == 2) {
			extension = args[1];
		} else {
			std::map<std::string, std::string> all_cgi;
			all_cgi["/usr/bin/php-cgi"] = ".php";
			all_cgi["/usr/bin/python3"] = ".py";
			all_cgi["/usr/bin/python2"] = ".py";
			all_cgi["/usr/bin/perl"] = ".pl";
			all_cgi["/usr/bin/ruby"] = ".rb";
			all_cgi["/bin/bash"] = ".sh";
			all_cgi["/usr/bin/node"] = ".js";

			std::map<std::string, std::string>::iterator it = all_cgi.find(app);
			if (it == all_cgi.end()) {
				throw std::runtime_error(
					"Error: 'cgi' directive requires known interpreter or "
					"extension (.php/.py/.pl/.rb/.sh/.js)");
			}
			extension = it->second;
		}

		config.cgi_handlers[extension] = app;
	}
};

class ListenDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1) {
			throw std::runtime_error(
				"Error: 'listen' directive requires exactly one argument");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);

		const std::string& value = args[0];
		const size_t colonPos = value.find(":");

		if (colonPos != std::string::npos) {
			config.host = value.substr(0, colonPos);
			if (config.host == "localhost") {
				config.host = "127.0.0.1";
			} else {
				std::istringstream stream(config.host);
				std::string part;
				int count = 0;
				while (std::getline(stream, part, '.')) {
					if (part.empty()) {
						break;
					}
					count++;
					if (part == "0") {
						continue;
					}
					const int nbr = std::atoi(part.c_str());
					if (nbr < 1 || nbr > 255) {
						throw std::runtime_error("Host not valid");
					}
				}
				if (count != 4) {
					throw std::runtime_error("Host not valid");
				}
			}

			config.port = parseInt(value.substr(colonPos + 1));
		} else {
			config.port = parseInt(value);
		}

		if (config.port <= 0 || config.port > 65535) {
			throw std::runtime_error(
				"Error: Invalid port number (must be between 1 and 65535)");
		}
	}
};

class MaxConnectionsDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1) {
			throw std::runtime_error(
				"Error: 'max_connections' directive requires exactly one "
				"argument");
		}
		if (!isNumber(args[0])) {
			throw std::runtime_error(
				"Error: Invalid value for 'max_connections' directive");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		try {
			config.max_connections = conversionBytesParsing(args[0]);
		} catch (...) {
			throw std::runtime_error(
				"Error: Invalid value for 'max_connections' directive");
		}
	}
};

class SessionTimeoutDirective : public IDirectiveHandler {
   public:
	void validate(const std::vector<std::string>& args) const {
		if (args.size() != 1) {
			throw std::runtime_error(
				"Error: 'session_timeout' directive requires exactly one "
				"argument");
		}
		if (!isNumber(args[0])) {
			throw std::runtime_error(
				"Error: Invalid value for 'session_timeout' directive");
		}
	}

	void apply(Config& config, const std::vector<std::string>& args) const {
		validate(args);
		try {
			config.session_timeout = conversionBytesParsing(args[0]);
		} catch (...) {
			throw std::runtime_error(
				"Error: Invalid value for 'session_timeout' directive");
		}
	}
};

}  // namespace

DirectiveRegistry::DirectiveRegistry() {
	registerDirective(SCOPE_GLOBAL, "root", new RootDirective());
	registerDirective(SCOPE_GLOBAL, "index", new IndexDirective());
	registerDirective(SCOPE_GLOBAL, "error_page", new ErrorPageDirective());
	registerDirective(SCOPE_GLOBAL, "autoindex", new AutoIndexDirective());
	registerDirective(SCOPE_GLOBAL, "max_connections",
					  new MaxConnectionsDirective());
	registerDirective(SCOPE_GLOBAL, "session_timeout",
					  new SessionTimeoutDirective());
	registerDirective(SCOPE_GLOBAL, "client_max_body_size",
					  new ClientMaxBodySizeDirective());
	registerDirective(SCOPE_GLOBAL, "large_client_header_buffers",
					  new LargeClientHeaderBuffersDirective());
	registerDirective(SCOPE_GLOBAL, "client_header_buffer_size",
					  new ClientHeaderBufferSizeDirective());

	registerDirective(SCOPE_SERVER, "listen", new ListenDirective());
	registerDirective(SCOPE_SERVER, "root", new RootDirective());
	registerDirective(SCOPE_SERVER, "index", new IndexDirective());
	registerDirective(SCOPE_SERVER, "error_page", new ErrorPageDirective());
	registerDirective(SCOPE_SERVER, "autoindex", new AutoIndexDirective());
	registerDirective(SCOPE_SERVER, "client_max_body_size",
					  new ClientMaxBodySizeDirective());
	registerDirective(SCOPE_SERVER, "large_client_header_buffers",
					  new LargeClientHeaderBuffersDirective());
	registerDirective(SCOPE_SERVER, "client_header_buffer_size",
					  new ClientHeaderBufferSizeDirective());
	registerDirective(SCOPE_SERVER, "return", new ReturnDirective());
	registerDirective(SCOPE_SERVER, "allow_methods",
					  new AllowMethodsDirective());
	registerDirective(SCOPE_SERVER, "upload_store", new UploadStoreDirective());
	registerDirective(SCOPE_SERVER, "cgi", new CgiDirective());

	registerDirective(SCOPE_LOCATION, "root", new RootDirective());
	registerDirective(SCOPE_LOCATION, "index", new IndexDirective());
	registerDirective(SCOPE_LOCATION, "error_page", new ErrorPageDirective());
	registerDirective(SCOPE_LOCATION, "autoindex", new AutoIndexDirective());
	registerDirective(SCOPE_LOCATION, "client_max_body_size",
					  new ClientMaxBodySizeDirective());
	registerDirective(SCOPE_LOCATION, "return", new ReturnDirective());
	registerDirective(SCOPE_LOCATION, "upload_store",
					  new UploadStoreDirective());
	registerDirective(SCOPE_LOCATION, "allow_methods",
					  new AllowMethodsDirective());
	registerDirective(SCOPE_LOCATION, "cgi", new CgiDirective());
}

DirectiveRegistry::~DirectiveRegistry() {
	for (size_t i = 0; i < _ownedHandlers.size(); ++i) {
		delete _ownedHandlers[i];
	}
	_ownedHandlers.clear();
}

void DirectiveRegistry::registerDirective(Scope scope, const std::string& name,
										  const IDirectiveHandler* handler) {
	if (!handler) {
		throw std::runtime_error("Error: Null directive handler");
	}
	HandlerMap* map = NULL;
	if (scope == SCOPE_GLOBAL) {
		map = &_globalHandlers;
	} else if (scope == SCOPE_SERVER) {
		map = &_serverHandlers;
	} else {
		map = &_locationHandlers;
	}

	(*map)[name] = handler;
	_ownedHandlers.push_back(handler);
}

const IDirectiveHandler* DirectiveRegistry::getHandler(
	Scope scope, const std::string& name) const {
	const HandlerMap* map = NULL;
	if (scope == SCOPE_GLOBAL) {
		map = &_globalHandlers;
	} else if (scope == SCOPE_SERVER) {
		map = &_serverHandlers;
	} else {
		map = &_locationHandlers;
	}

	HandlerMap::const_iterator it = map->find(name);
	if (it == map->end()) {
		return NULL;
	}
	return it->second;
}

bool DirectiveRegistry::has(Scope scope, const std::string& name) const {
	return getHandler(scope, name) != NULL;
}

void DirectiveRegistry::apply(Scope scope, const std::string& name,
							  Config& config,
							  const std::vector<std::string>& args) const {
	const IDirectiveHandler* handler = getHandler(scope, name);
	if (!handler) {
		throw std::runtime_error("Error: Unknown directive '" + name + "'");
	}
	handler->apply(config, args);
}
