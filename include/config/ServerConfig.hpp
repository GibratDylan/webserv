/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:47:39 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/04 18:32:05 by dgibrat          ###   ########.fr       */
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
	ServerConfig(const std::string& serverDirective, const Config &globalConfig);
	ServerConfig(const ServerConfig& src);
	~ServerConfig();

	ServerConfig& operator=(ServerConfig const& rhs);

	const std::string& getDirective(const std::string& interface,
									const std::string& location,
									const std::string& name) const;

   private:
	void handleLocation(const std::string& serverDirective);

   public:
	std::map<std::string, Config*> location;
};

#endif
