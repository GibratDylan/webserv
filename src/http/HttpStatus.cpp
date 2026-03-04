/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 14:36:20 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/04 14:55:18 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/HttpStatus.hpp"

std::string HttpStatus::getMessage(int code) {
	if (!isValid(code)) {
		return "Unknown status code";
	}
	return _messages.find(code)->second;
}

std::map<int, std::string> HttpStatus::initMessages() {
	std::map<int, std::string> status;

	// 1xx - Réponses informatives
	status[100] = "Continue";
	status[101] = "Switching Protocols";
	status[102] = "Processing";
	status[103] = "Early Hints";

	// 2xx - Réponses de succès
	status[200] = "OK";
	status[201] = "Created";
	status[202] = "Accepted";
	status[203] = "Non-Authoritative Information";
	status[204] = "No Content";
	status[205] = "Reset Content";
	status[206] = "Partial Content";
	status[207] = "Multi-Status";
	status[208] = "Already Reported";
	status[226] = "IM Used";

	// 3xx - Messages de redirection
	status[300] = "Multiple Choices";
	status[301] = "Moved Permanently";
	status[302] = "Found";
	status[303] = "See Other";
	status[304] = "Not Modified";
	status[307] = "Temporary Redirect";
	status[308] = "Permanent Redirect";

	// 4xx - Erreurs côté client
	status[400] = "Bad Request";
	status[401] = "Unauthorized";
	status[402] = "Payment Required";
	status[403] = "Forbidden";
	status[404] = "Not Found";
	status[405] = "Method Not Allowed";
	status[406] = "Not Acceptable";
	status[407] = "Proxy Authentication Required";
	status[408] = "Request Timeout";
	status[409] = "Conflict";
	status[410] = "Gone";
	status[411] = "Length Required";
	status[412] = "Precondition Failed";
	status[413] = "Content Too Large";
	status[414] = "URI Too Long";
	status[415] = "Unsupported Media Type";
	status[416] = "Range Not Satisfiable";
	status[417] = "Expectation Failed";
	status[418] = "I'm a teapot";
	status[421] = "Misdirected Request";
	status[422] = "Unprocessable Content";
	status[423] = "Locked";
	status[424] = "Failed Dependency";
	status[425] = "Too Early";
	status[426] = "Upgrade Required";
	status[428] = "Precondition Required";
	status[429] = "Too Many Requests";
	status[431] = "Request Header Fields Too Large";
	status[451] = "Unavailable For Legal Reasons";

	// 5xx - Erreurs côté serveur
	status[500] = "Internal Server Error";
	status[501] = "Not Implemented";
	status[502] = "Bad Gateway";
	status[503] = "Service Unavailable";
	status[504] = "Gateway Timeout";
	status[505] = "HTTP Version Not Supported";
	status[506] = "Variant Also Negotiates";
	status[507] = "Insufficient Storage";
	status[508] = "Loop Detected";
	status[510] = "Not Extended";
	status[511] = "Network Authentication Required";

	return status;
}

bool HttpStatus::isValid(int code) {
	return _messages.find(code) != _messages.end();
}

bool HttpStatus::isSuccess(int code) {
	return (isValid(code) && code >= 200 && code < 300);
}

bool HttpStatus::isRedirection(int code) {
	return (isValid(code) && code >= 300 && code < 400);
}

bool HttpStatus::isClientError(int code) {
	return (isValid(code) && code >= 400 && code < 500);
}

bool HttpStatus::isServerError(int code) {
	return (isValid(code) && code >= 500 && code < 600);
}

std::map<int, std::string> HttpStatus::_messages = HttpStatus::initMessages();
