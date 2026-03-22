/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseBuilder.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/HttpResponseBuilder.hpp"

#include "../../include/config/Config.hpp"
#include "../../include/server/utils.hpp"

HttpResponseBuilder::HttpResponseBuilder()
	: _statusCode(200), _reason(""), _headers(), _body() {}

HttpResponseBuilder& HttpResponseBuilder::setStatus(int code) {
	_statusCode = code;
	_reason = HttpResponse::getReason(code);
	return *this;
}

HttpResponseBuilder& HttpResponseBuilder::setHeader(const std::string& key,
													const std::string& value) {
	_headers[key] = value;
	return *this;
}

HttpResponseBuilder& HttpResponseBuilder::setBody(const std::string& body) {
	_body = body;
	return *this;
}

HttpResponseBuilder& HttpResponseBuilder::setContentType(
	const std::string& type) {
	_headers["Content-Type"] = type;
	return *this;
}

HttpResponse HttpResponseBuilder::build() const {
	HttpResponse res(_statusCode, _reason.empty()
									  ? HttpResponse::getReason(_statusCode)
									  : _reason);
	res.headers = _headers;
	res.body = _body;
	if (!res.headers.count("Content-Length")) {
		res.headers["Content-Length"] = toString(res.body.size());
	}
	if (!res.headers.count("Connection")) {
		res.headers["Connection"] = "close";
	}
	return res;
}

HttpResponse HttpResponseBuilder::makeError(int code,
											const Config& config) const {
	return HttpResponse::makeErrorResponse(code, config);
}

HttpResponse HttpResponseBuilder::makeRedirect(
	int code, const std::string& location) const {
	return HttpResponse::makeRedirectResponse(code, location);
}
