/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/HttpResponse.hpp"

#include <sstream>

#include "../../include/config/Config.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utility/MimeTypeResolver.hpp"
#include "../../include/utility/PathUtils.hpp"

std::map<int, std::string> HttpResponse::reasons;

HttpResponse::HttpResponse() : statusCode(200), reason("OK") {}

HttpResponse::HttpResponse(int code, const std::string& reason)
	: statusCode(code), reason(reason) {}

HttpResponse::HttpResponse(const HttpResponse& other)
	: statusCode(other.statusCode),
	  reason(other.reason),
	  headers(other.headers),
	  body(other.body) {}

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

	for (HeaderMap::const_iterator it = headers.begin(); it != headers.end();
		 ++it)
		response << it->first << ": " << it->second << "\r\n";

	response << "\r\n";
	response << body;

	return response.str();
}

// Static

void HttpResponse::initReasons() {
	if (!reasons.empty()) return;

	// 1xx - Informational
	reasons[100] = "Continue";
	reasons[101] = "Switching Protocols";
	reasons[102] = "Processing";
	reasons[103] = "Early Hints";

	// 2xx - Success
	reasons[200] = "OK";
	reasons[201] = "Created";
	reasons[202] = "Accepted";
	reasons[203] = "Non-Authoritative Information";
	reasons[204] = "No Content";
	reasons[205] = "Reset Content";
	reasons[206] = "Partial Content";
	reasons[207] = "Multi-Status";
	reasons[208] = "Already Reported";
	reasons[226] = "IM Used";

	// 3xx - Redirection
	reasons[300] = "Multiple Choices";
	reasons[301] = "Moved Permanently";
	reasons[302] = "Found";
	reasons[303] = "See Other";
	reasons[304] = "Not Modified";
	reasons[307] = "Temporary Redirect";
	reasons[308] = "Permanent Redirect";

	// 4xx - Client Error
	reasons[400] = "Bad Request";
	reasons[401] = "Unauthorized";
	reasons[402] = "Payment Required";
	reasons[403] = "Forbidden";
	reasons[404] = "Not Found";
	reasons[405] = "Method Not Allowed";
	reasons[406] = "Not Acceptable";
	reasons[407] = "Proxy Authentication Required";
	reasons[408] = "Request Timeout";
	reasons[409] = "Conflict";
	reasons[410] = "Gone";
	reasons[411] = "Length Required";
	reasons[412] = "Precondition Failed";
	reasons[413] = "Content Too Large";
	reasons[414] = "URI Too Long";
	reasons[415] = "Unsupported Media Type";
	reasons[416] = "Range Not Satisfiable";
	reasons[417] = "Expectation Failed";
	reasons[418] = "I'm a teapot";
	reasons[421] = "Misdirected Request";
	reasons[422] = "Unprocessable Content";
	reasons[423] = "Locked";
	reasons[424] = "Failed Dependency";
	reasons[425] = "Too Early";
	reasons[426] = "Upgrade Required";
	reasons[428] = "Precondition Required";
	reasons[429] = "Too Many Requests";
	reasons[431] = "Request Header Fields Too Large";
	reasons[451] = "Unavailable For Legal Reasons";

	// 5xx - Server Error
	reasons[500] = "Internal Server Error";
	reasons[501] = "Not Implemented";
	reasons[502] = "Bad Gateway";
	reasons[503] = "Service Unavailable";
	reasons[504] = "Gateway Timeout";
	reasons[505] = "HTTP Version Not Supported";
	reasons[506] = "Variant Also Negotiates";
	reasons[507] = "Insufficient Storage";
	reasons[508] = "Loop Detected";
	reasons[510] = "Not Extended";
	reasons[511] = "Network Authentication Required";
}

bool HttpResponse::isValid(int code) {
	if (reasons.empty()) initReasons();
	return reasons.find(code) != reasons.end();
}

bool HttpResponse::isSuccess(int code) {
	return (isValid(code) && code >= 200 && code < 300);
}

bool HttpResponse::isRedirection(int code) {
	return (isValid(code) && code >= 300 && code < 400);
}

bool HttpResponse::isClientError(int code) {
	return (isValid(code) && code >= 400 && code < 500);
}

bool HttpResponse::isServerError(int code) {
	return (isValid(code) && code >= 500 && code < 600);
}

std::string HttpResponse::getReason(int code) {
	if (reasons.empty()) initReasons();

	std::map<int, std::string>::const_iterator it = reasons.find(code);
	if (it == reasons.end()) {
		Logger::warning(std::string(" Unknown HTTP status code requested: ") +
						toString(code));
		return std::string("Error ") + toString(code);
	}
	return it->second;
}

HttpResponse HttpResponse::makeResponse(int code, const std::string& type,
										const std::string& body) {
	HttpResponse res(code, getReason(code));

	Logger::info(std::string(" Response ready code=") + toString(code) +
				 " body_bytes=" + toString(body.size()));

	res.body = body;
	res.headers["Content-Length"] = toString(res.body.size());
	res.headers["Content-Type"] = type.empty() ? "text/plain" : type;
	res.headers["Connection"] = "close";

	return res;
}

HttpResponse HttpResponse::makeErrorResponse(int code, const Config& config) {
	Logger::debug(std::string(" makeErrorResponse code=") + toString(code));

	std::string body;
	std::string type;
	bool hasBody = false;

	std::map<int, std::string>::const_iterator it =
		config.error_pages.find(code);
	if (it != config.error_pages.end()) {
		std::string errPagePath = addPath(
			config.root,
			PathUtils::normalizeForLocation(it->second, config.location_path));

		if (FileSystem::exists(errPagePath)) {
			Logger::debug(std::string(" using custom error page ") +
						  errPagePath);
			try {
				body = FileSystem::readFile(errPagePath);
				type = "text/html";
				hasBody = true;
			} catch (const std::exception& err) {
				// fall through to plain-text below
			}
		}
	}

	if (!hasBody) {
		std::string defaultPath =
			"./assets/error_pages/" + toString(code) + ".html";
		if (FileSystem::exists(defaultPath)) {
			Logger::debug(std::string(" using default error page ") +
						  defaultPath);
			try {
				body = FileSystem::readFile(defaultPath);
				type = "text/html";
				hasBody = true;
			} catch (const std::exception& err) {
				// fall through to plain-text below
			}
		}
	}

	if (!hasBody) {
		body = toString(code) + " " + getReason(code);
		type = "text/plain";
	}

	HttpResponse res = HttpResponse::makeResponse(code, type, body);
	if (code == 405 && !config.methods.empty()) {
		std::string allow;
		for (size_t i = 0; i < config.methods.size(); ++i) {
			if (!allow.empty()) allow += ", ";
			allow += config.methods[i];
		}
		if (!allow.empty()) res.headers["Allow"] = allow;
	}
	return res;
}

HttpResponse HttpResponse::makeRedirectResponse(int code,
												const std::string& url) {
	HttpResponse res(code, getReason(code));
	res.headers["Connection"] = "close";
	if (code >= 300 && code < 400) {
		res.headers["Location"] = url;
		res.headers["Content-Length"] = "0";
	} else {
		res.headers["Content-Length"] = toString(url.size());
		res.headers["Content-Type"] = "text/plain";
		res.body = url;
	}
	return res;
}

HttpResponse HttpResponse::makeGetResponse(const std::string& path,
										   const Config& config) {
	std::string safePath =
		PathUtils::normalizeForLocation(path, config.location_path);

	std::string rootPath = addPath(config.root, safePath);

	if (!FileSystem::isDirectory(rootPath)) {
		return makeFileResponse(rootPath, config);
	}

	{
		Logger::debug(std::string(" directory requested ") + rootPath);

		std::string indexPath;
		std::string indexName;
		if (FileSystem::findIndexFile(rootPath, config.index, indexPath,
									  indexName)) {
			return makeFileResponse(indexPath, config);
		}

		if (config.autoindex) {
			std::string html =
				FileSystem::generateAutoIndex(rootPath, safePath);
			return HttpResponse::makeResponse(200, "text/html", html);
		}
		return HttpResponse::makeErrorResponse(403, config);
	}
}

HttpResponse HttpResponse::makeFileResponse(const std::string& path,
											const Config& config) {
	if (FileSystem::exists(path)) {
		try {
			std::string content = FileSystem::readFile(path);
			return HttpResponse::makeResponse(
				200, MimeTypeResolver::resolve(path), content);
		} catch (const std::exception& err) {
			return HttpResponse::makeErrorResponse(403, config);
		}
	}
	return HttpResponse::makeErrorResponse(404, config);
}

HttpResponse HttpResponse::makeDeleteResponse(const std::string& path,
											  const Config& config) {
	std::string safePath =
		PathUtils::normalizeForLocation(path, config.location_path);

	std::string rootPath = addPath(config.upload_store, safePath);

	if (!FileSystem::exists(rootPath)) {
		return HttpResponse::makeErrorResponse(404, config);
	}
	if (!FileSystem::isWritable(rootPath)) {
		return HttpResponse::makeErrorResponse(403, config);
	}

	if (FileSystem::deleteFile(rootPath)) {
		HttpResponse res(204, getReason(204));
		res.headers["Content-Length"] = "0";
		res.headers["Connection"] = "close";
		return res;
	}

	return HttpResponse::makeErrorResponse(403, config);
}

HttpResponse HttpResponse::makePostResponse(const std::string& path,
											const std::string& body,
											const Config& config) {
	if (body.size() > config.client_max_body_size) {
		return HttpResponse::makeErrorResponse(413, config);
	}

	std::string safePath =
		PathUtils::normalizeForLocation(path, config.location_path);
	Logger::debug(std::string(" makePostResponse safePath ") + safePath);

	std::string uploadPath = addPath(config.upload_store, safePath);

	if (FileSystem::isDirectory(uploadPath))
		uploadPath = addPath(uploadPath, path);

	if (FileSystem::writeFile(uploadPath, body)) {
		HttpResponse res(201, getReason(201));
		res.headers["Content-Length"] = "0";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Connection"] = "close";

		return res;
	}

	return HttpResponse::makeErrorResponse(403, config);
}
