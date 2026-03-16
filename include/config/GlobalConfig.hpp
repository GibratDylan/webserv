/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/16 09:41:24 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GLOBALCONFIG_HPP
#define GLOBALCONFIG_HPP

#include <map>
#include <string>

#include "Config.hpp"

class ServerConfig;

class GlobalConfig : public Config {
   public:
	GlobalConfig(const std::string& pathConfigFile);
	GlobalConfig(const GlobalConfig& src);
	~GlobalConfig();

	GlobalConfig& operator=(const GlobalConfig& rhs);

	std::string printDirectives() const;

   private:
	void parseGlobalDirective(const std::string& allDirective);

	size_t handleServer(const std::string& serverDirective);

   public:
	std::map<int, ServerConfig> server;
};

#endif
