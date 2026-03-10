/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:47:39 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/10 17:09:15 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include <string>

#include "Config.hpp"

class LocationConfig;

class ServerConfig : public Config {
   public:
	ServerConfig(const std::string& serverDirective, const Config& globalConfig);
	ServerConfig(const ServerConfig& src);
	~ServerConfig();

	ServerConfig& operator=(ServerConfig const& rhs);

	Config* resolveConfig(const std::string& location);

   private:
	void parseServerDirective(const std::string& serverDirective);
	size_t handleLocation(const std::string& locationDirective, const std::string& pathLocation);

   public:
	std::map<std::string, Config*> location;
};

#endif
