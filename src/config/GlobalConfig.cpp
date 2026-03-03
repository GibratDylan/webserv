/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 23:24:23 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/GlobalConfig.hpp"

#include <iostream>
#include <list>
#include <sstream>

GlobalConfig::GlobalConfig(const std::string& directive) {
	const char* delimiters = " \t;";

	std::istringstream input;
	input.str(directive);

	std::string line;
	std::list<std::string> words;

	while (static_cast<bool>(std::getline(input, line))) {
		words.clear();

		size_t comment_pos = line.find('#');
		if (comment_pos != std::string::npos) {
			line = line.substr(0, comment_pos);
		}

		if (line.empty()) {
			continue;
		}

		size_t semicolon_pos = line.find(';');
		if (semicolon_pos == std::string::npos) {
			throw std::exception();
		}

		while (semicolon_pos < line.length() &&
			   std::string(delimiters).find(line.at(semicolon_pos)) !=
				   std::string::npos &&
			   line.at(semicolon_pos) != ';') {
			semicolon_pos++;
		}

		if (semicolon_pos + 1 != line.length()) {
			throw std::exception();
		}

		size_t pos = 0;
		while (pos < line.length()) {
			while (pos < line.length() &&
				   std::string(delimiters).find(line.at(pos)) !=
					   std::string::npos) {
				pos++;
			}

			size_t end = pos + 1;
			while (end < line.length() &&
				   std::string(delimiters).find(line.at(end)) ==
					   std::string::npos) {
				end++;
			}

			words.push_back(line.substr(pos, end - pos));

			pos = end;
		}

		if (!words.empty()) {
			std::string key = words.front();
			words.pop_front();
			_directive[key] =
				std::vector<std::string>(words.begin(), words.end());
		}
	}
}

GlobalConfig::GlobalConfig(const GlobalConfig& src)
	: _directive(src._directive), _server(src._server) {}

GlobalConfig::~GlobalConfig() {}

GlobalConfig& GlobalConfig::operator=(const GlobalConfig& rhs) {
	if (this != &rhs) {
		_directive = rhs._directive;
		_server = rhs._server;
	}
	return *this;
}

void GlobalConfig::printDirectives() const {
	for (std::map<std::string, std::vector<std::string> >::const_iterator it =
			 _directive.begin();
		 it != _directive.end(); ++it) {
		std::cout << "Directive: " << it->first << '\n';
		std::cout << "  Values: ";
		for (std::vector<std::string>::const_iterator val_it =
				 it->second.begin();
			 val_it != it->second.end(); ++val_it) {
			if (val_it != it->second.begin()) {
				std::cout << ", ";
			}
			std::cout << *val_it;
		}
		std::cout << "\n\n";
	}
}

// const std::string& GlobalConfig::getDirective(const std::string& interface,
// 											  const std::string& location,
// 											  const std::string& name) {
// 	struct DirectiveParams {
// 		const std::string& interface;
// 		const std::string& location;
// 		const std::string& name;
// 	};

// 	DirectiveParams params = {interface, location, name};
// 	(void)params;
// 	static std::string empty;
// 	return empty;
// }
