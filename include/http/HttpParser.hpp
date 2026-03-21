/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <cstddef>
#include <string>

#include "../server/HttpRequest.hpp"
#include "HttpRequestValidator.hpp"

class Config;
class ServerConfig;

class HttpParser {
   public:
	explicit HttpParser(const ServerConfig& serverConfig);

	ParseStatus parse(const std::string& buffer, HttpRequest& request);
	void reset();
	const Config* getResolvedConfig() const;

   private:
	ParseStatus parseHeaders(const std::string& buffer, HttpRequest& request);
	ParseStatus parseChunkedBody(const std::string& buffer, size_t headerEnd,
								 HttpRequest& request,
								 const Config& resolvedConfig);
	ParseStatus parseContentLengthBody(const std::string& buffer,
									   size_t headerEnd, HttpRequest& request,
									   const Config& resolvedConfig);

   private:
	const ServerConfig& _serverConfig;
	HttpRequestValidator _validator;

	bool _headersParsed;
	size_t _headerEnd;
	const Config* _resolvedConfig;
};

#endif
