/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestValidator.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/HttpRequestValidator.hpp"

#include <algorithm>
#include <cstdlib>

#include "../../include/config/Config.hpp"
#include "../../include/server/utils.hpp"

HttpRequestValidator::HttpRequestValidator() {}

ParseStatus HttpRequestValidator::validateMethod(
	const std::string& method, const Config& resolvedConfig) const {
	if (std::find(resolvedConfig.methods.begin(), resolvedConfig.methods.end(),
				  method) == resolvedConfig.methods.end())
		return METHOD_NOT_ALLOWED;
	return PARSE_OK;
}

ParseStatus HttpRequestValidator::validateUriLength(const std::string& uri,
													size_t maxBytes) const {
	if (uri.size() > maxBytes) return URI_TOO_LONG;
	return PARSE_OK;
}

ParseStatus HttpRequestValidator::validateVersion(
	const std::string& version) const {
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		return HTTP_VERSION_NOT_SUPPORTED;
	return PARSE_OK;
}

ParseStatus HttpRequestValidator::validateHeaderSize(size_t headerBytes,
													 size_t maxBytes) const {
	if (headerBytes > maxBytes) return HEADER_FIELDS_TOO_LARGE;
	return PARSE_OK;
}

ParseStatus HttpRequestValidator::validateBodyFraming(
	const std::string& method,
	const std::map<std::string, std::string>& headers) const {
	if ((method == "POST" || method == "PUT") &&
		!headers.count("Content-Length") &&
		!(headers.count("Transfer-Encoding") &&
		  headers.find("Transfer-Encoding")->second == "chunked")) {
		return LENGTH_REQUIRED;
	}
	return PARSE_OK;
}

ParseStatus HttpRequestValidator::validateContentLength(
	const std::map<std::string, std::string>& headers, size_t maxBodyBytes,
	size_t& outLength) const {
	outLength = 0;
	std::map<std::string, std::string>::const_iterator it =
		headers.find("Content-Length");
	if (it == headers.end()) return PARSE_OK;

	const std::string& lenStr = it->second;
	if (!isNumber(lenStr)) return BAD_REQUEST;

	outLength = static_cast<size_t>(std::strtoul(lenStr.c_str(), NULL, 10));
	if (outLength > maxBodyBytes) return PAYLOAD_TOO_LARGE;
	return PARSE_OK;
}
