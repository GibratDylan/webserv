/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 10:14:45 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/14 12:37:20 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

#include <iostream>

class Logger {
   public:
	enum Level { DEBUG, INFO, WARNING, ERROR, NONE };

   private:
	static Level _level;
	static std::ostream* _output;
	static bool _timestamps;

   public:
	static void setLevel(Level level) { _level = level; }

	static void setTimestamps(bool enable) { _timestamps = enable; }

	static void debug(const std::string& message);
	static void info(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& message);

	static bool isDebugEnabled() { return _level <= DEBUG; }

   private:
	Logger();

	static std::string getCurrentTime();
};

#endif	// LOGGER_HPP
