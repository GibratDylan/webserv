/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiResponseParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Copilot <copilot@openai.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 11:00:00 by Copilot           #+#    #+#             */
/*   Updated: 2026/04/02 11:00:00 by Copilot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSEPARSER_HPP
#define CGIRESPONSEPARSER_HPP

#include <map>
#include <string>

class CgiResponseParser {
   public:
	struct ParsedResponse {
		int code;
		std::string type;
		std::string body;
		std::map<std::string, std::string> headers;

		ParsedResponse() : code(200), type("text/html"), body(), headers() {}
	};

	ParsedResponse parse(const std::string& output) const;
};

#endif
