/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestValidator.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTVALIDATOR_HPP
#define HTTPREQUESTVALIDATOR_HPP

#include <cstddef>
#include <map>
#include <string>

#include "../server/HttpRequest.hpp"

class Config;

class HttpRequestValidator {
   public:
	HttpRequestValidator();

	ParseStatus validateMethod(const std::string& method,
							   const Config& resolvedConfig) const;
	ParseStatus validateUriLength(const std::string& uri,
								  size_t maxBytes) const;
	ParseStatus validateVersion(const std::string& version) const;
	ParseStatus validateHeaderSize(size_t headerBytes, size_t maxBytes) const;
	ParseStatus validateBodyFraming(
		const std::string& method,
		const std::map<std::string, std::string>& headers) const;
	ParseStatus validateContentLength(
		const std::map<std::string, std::string>& headers, size_t maxBodyBytes,
		size_t& outLength) const;
};

#endif
