/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiResponseParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Copilot <copilot@openai.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 11:00:00 by Copilot           #+#    #+#             */
/*   Updated: 2026/04/02 11:00:00 by Copilot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cgi/CgiResponseParser.hpp"

#include <cctype>
#include <sstream>

CgiResponseParser::ParsedResponse CgiResponseParser::parse(
	const std::string& output) const {
	ParsedResponse parsed;

	if (output.empty()) {
		parsed.code = 500;
		parsed.body = "CGI script produced no output";
		return parsed;
	}

	size_t headerEnd = output.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		headerEnd = output.find("\n\n");
		if (headerEnd != std::string::npos) {
			parsed.body = output.substr(headerEnd + 2);
		} else {
			parsed.body = output;
			parsed.code = 200;
			return parsed;
		}
	} else {
		parsed.body = output.substr(headerEnd + 4);
	}

	std::string headerSection = output.substr(0, headerEnd);
	std::istringstream stream(headerSection);
	std::string line;

	parsed.code = 200;

	while (std::getline(stream, line)) {
		if (line.empty() || line == "\r") {
			break;
		}

		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) {
			continue;
		}

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos) {
			value = value.substr(start);
		}
		size_t end = value.find_last_not_of(" \t");
		if (end != std::string::npos) {
			value = value.substr(0, end + 1);
		}

		std::string lowerKey = key;
		for (size_t i = 0; i < lowerKey.size(); ++i) {
			lowerKey[i] = static_cast<char>(
				std::tolower(static_cast<unsigned char>(lowerKey[i])));
		}

		if (lowerKey == "status") {
			std::istringstream codeStream(value);
			codeStream >> parsed.code;
		} else if (lowerKey == "content-type") {
			parsed.type = value;
		}

		parsed.headers[key] = value;
	}

	return parsed;
}
