/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:47:39 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:12 by dgibrat          ###   ########.fr       */
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
	ServerConfig(const Config& globalConfig);
	ServerConfig(const ServerConfig& src);
	~ServerConfig();

	ServerConfig& operator=(ServerConfig const& rhs);

	const Config& resolveConfig(const std::string& location) const;

   public:
	std::map<std::string, Config> location;
};

#endif
