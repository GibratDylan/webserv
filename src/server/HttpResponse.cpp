#include "../../include/HttpResponse.h"

#include <iostream>
#include <sstream>

#include "../../include/FileHandler.h"
#include "../../include/config/Config.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utils.h"

std::map<int, std::string> HttpResponse::reasons;

HttpResponse::HttpResponse() : statusCode(200), reason("OK") {}

HttpResponse::HttpResponse(int code, const std::string& reason) : statusCode(code), reason(reason) {}

HttpResponse::HttpResponse(const HttpResponse& other) : statusCode(other.statusCode), reason(other.reason), headers(other.headers), body(other.body) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		statusCode = other.statusCode;
		reason = other.reason;
		headers = other.headers;
		body = other.body;
	}
	return *this;
}

void HttpResponse::addHeader(const std::string& key, const std::string& value) {
	headers[key] = value;
}

std::string HttpResponse::build() const {
	std::ostringstream response;

	response << "HTTP/1.1 " << statusCode << " " << reason << "\r\n";

	for (HeaderMap::const_iterator it = headers.begin(); it != headers.end(); ++it) response << it->first << ": " << it->second << "\r\n";

	response << "\r\n";
	response << body;

	return response.str();
}

// Static

void HttpResponse::initReasons() {
	reasons[200] = "OK";
	reasons[201] = "Created";
	reasons[204] = "No Content";
	reasons[301] = "Moved Permanently";
	reasons[302] = "Found";
	reasons[303] = "See Other";
	reasons[307] = "Temporary Redirect";
	reasons[308] = "Permanent Redirect";
	reasons[400] = "Bad Request";
	reasons[403] = "Forbidden";
	reasons[404] = "Not Found";
	reasons[405] = "Method Not Allowed";
	reasons[409] = "Conflict";
	reasons[411] = "Length Required";
	reasons[413] = "Payload Too Large";
	reasons[414] = "URI Too Long";
	reasons[431] = "Header Too Long";
	reasons[500] = "Internal Server Error";
	reasons[501] = "Not Implemented";
	reasons[504] = "Gateway Timeout";
}

std::string HttpResponse::getReason(int code) {
	return reasons.find(code) != reasons.end() ? reasons[code] : std::string("Error ") + toString(code);
}

HttpResponse HttpResponse::makeResponse(int code, const std::string& type, const std::string& body) {
	HttpResponse res(code, getReason(code));

	Logger::info(std::string(" Response ready code=") + toString(code) + " body_bytes=" + toString(body.size()));

	res.body = body;
	res.headers["Content-Length"] = toString(res.body.size());
	res.headers["Content-Type"] = type.empty() ? "text/plain" : type;
	res.headers["Connection"] = "close";

	return res;
}

HttpResponse HttpResponse::makeErrorResponse(int code, const Config* config) {
	Logger::debug(std::string(" makeErrorResponse code=") + toString(code));

	std::map<int, std::string>::const_iterator it = config->error_pages.find(code);
	if (it != config->error_pages.end()) {
		std::string errPagePath = config->root + FileHandler::normalizePath(it->second, config->location_path);

		if (FileSystem::exists(errPagePath)) {
			Logger::debug(std::string(" using custom error page ") + errPagePath);
			try {
				std::string content = FileSystem::readFile(errPagePath);
				return HttpResponse::makeResponse(code, "text/html", content);
			} catch (const std::exception& err) {
				return HttpResponse::makeResponse(code, "text/plain", toString(code) + " " + getReason(code));
			}
		}
	}

	std::string defaultPath = "./assets/error_pages/" + toString(code) + ".html";
	if (FileSystem::exists(defaultPath)) {
		Logger::debug(std::string(" using default error page ") + defaultPath);
		try {
			std::string content = FileSystem::readFile(defaultPath);
			return HttpResponse::makeResponse(code, "text/html", content);
		} catch (const std::exception& err) {
			return HttpResponse::makeResponse(code, "text/plain", toString(code) + " " + getReason(code));
		}
	}

	return HttpResponse::makeResponse(code, "text/plain", toString(code) + " " + getReason(code));
}

HttpResponse HttpResponse::makeRedirectResponse(int code, const std::string& str) {
	HttpResponse res(code, getReason(code));
	res.headers["Connection"] = "close";
	if (code >= 300 && code < 400) {
		res.headers["Location"] = str;
		res.headers["Content-Length"] = "0";
	} else {
		res.headers["Content-Length"] = toString(str.size());
		res.headers["Content-Type"] = "text/plain";
		res.body = str;
	}
	return res;
}

HttpResponse HttpResponse::makeGetResponse(const std::string& path, const Config* config) {
	std::string safePath = FileHandler::normalizePath(path, config->location_path);

	std::string rootPath = config->root + safePath;
	Logger::debug(std::string(" makeGetResponse path=") + rootPath);

	if (FileSystem::isDirectory(rootPath)) {
		Logger::debug(std::string(" directory requested ") + rootPath);

		std::string indexPath;
		for (size_t i = 0; i < config->index.size(); ++i) {
			indexPath = rootPath + "/" + config->index[i];
			if (FileSystem::exists(indexPath)) return makeFileResponse(indexPath, config);
		}

		if (config->autoindex) {
			std::string html = FileHandler::generateAutoIndex(rootPath, safePath);
			return HttpResponse::makeResponse(200, "text/html", html);
		} else
			return HttpResponse::makeErrorResponse(403, config);
	} else
		return makeFileResponse(rootPath, config);
}

HttpResponse HttpResponse::makeFileResponse(const std::string& path, const Config* config) {
	if (FileSystem::exists(path)) {
		try {
			std::string content = FileSystem::readFile(path);
			return HttpResponse::makeResponse(200, FileHandler::getMimeType(path), content);
		} catch (const std::exception& err) {
			return HttpResponse::makeErrorResponse(500, config);
		}
	}
	return HttpResponse::makeErrorResponse(404, config);
}

HttpResponse HttpResponse::makeDeleteResponse(const std::string& path, const Config* config) {
	Logger::debug(std::string(" makeDeleteResponse path=") + path);

	std::string safePath = FileHandler::normalizePath(path, config->location_path);

	std::string rootPath = config->upload_store + safePath;

	if (!FileSystem::exists(rootPath)) {
		return HttpResponse::makeErrorResponse(404, config);
	}

	if (FileSystem::deleteFile(rootPath)) {
		HttpResponse res(204, "No Content");
		res.headers["Content-Length"] = "0";
		res.headers["Connection"] = "close";
		return res;
	}
	return HttpResponse::makeErrorResponse(500, config);
}

HttpResponse HttpResponse::makePostResponse(const std::string& path, const std::string& body, const Config* config) {
	Logger::debug(std::string(" makePostResponse path=") + path + " body_bytes=" + toString(body.size()));

	if (body.size() > config->client_max_body_size) {
		return HttpResponse::makeErrorResponse(413, config);
	}

	std::string safePath = FileHandler::normalizePath(path, config->location_path);

	std::string uploadPath = config->upload_store + safePath;

	// if (FileSystem::isDirectory(uploadPath))
	//     return HttpResponse::makeErrorResponse(201, config);

	if (FileSystem::writeFile(uploadPath, body)) {
		HttpResponse res(201, "Created");
		res.headers["Content-Length"] = "0";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Connection"] = "close";

		return res;
	}

	return HttpResponse::makeErrorResponse(500, config);
}
