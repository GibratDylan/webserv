/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 10:14:45 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/14 12:38:08 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/Logger.hpp"

Logger::Level Logger::_level = Logger::ERROR;
std::ostream* Logger::_output = &std::cout;
bool Logger::_timestamps = true;


void Logger::debug(const std::string& message) {
	if (DEBUG < _level) {
		return;
	}

	if (_timestamps) {
		*_output << getCurrentTime();
	}

	*_output << COLOR_BLUE << "[DEBUG]" << message << COLOR_RESET << '\n';
}

void Logger::info(const std::string& message) {
	if (INFO < _level) {
		return;
	}

	if (_timestamps) {
		*_output << getCurrentTime();
	}

	*_output << COLOR_GREEN << "[INFO]" << message << COLOR_RESET << '\n';
}

void Logger::warning(const std::string& message) {
	if (WARNING < _level) {
		return;
	}

	if (_timestamps) {
		*_output << getCurrentTime();
	}

	*_output << COLOR_YELLOW << "[WARNING]" << message << COLOR_RESET << '\n';
}

void Logger::error(const std::string& message) {
	if (ERROR < _level) {
		return;
	}

	if (_timestamps) {
		*_output << getCurrentTime();
	}

	*_output << COLOR_RED << "[ERRROR]" << message << COLOR_RESET << '\n';
}

std::string Logger::getCurrentTime() {
	time_t now = time(0);
	struct tm tstruct = {};
	char buf[80];
	tstruct = *localtime(&now);

	strftime(buf, sizeof(buf), "[%Y-%m-%d.%X]", &tstruct);

	return buf;
}
