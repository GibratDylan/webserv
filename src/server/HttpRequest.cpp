/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/07 18:11:50 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/HttpRequest.hpp"

HttpRequest::HttpRequest()
	: method(), path(), query(), version(), headers(), body() {}

HttpRequest::HttpRequest(const HttpRequest& other)
	: method(other.method),
	  path(other.path),
	  query(other.query),
	  version(other.version),
	  headers(other.headers),
	  body(other.body) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other) {
		method = other.method;
		path = other.path;
		query = other.query;
		version = other.version;
		headers = other.headers;
		body = other.body;
	}
	return *this;
}

std::string HttpRequest::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(name);
	if (it != headers.end()) return it->second;
	return "";
}
