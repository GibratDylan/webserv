/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:11 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>

class GlobalConfig;

class ConfigParser {
   public:
	ConfigParser();
	~ConfigParser();

	GlobalConfig* parse(const std::string& filename) const;

   private:
	ConfigParser(const ConfigParser&);
	ConfigParser& operator=(const ConfigParser&);
};

#endif
