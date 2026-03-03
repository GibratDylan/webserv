/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:47:39 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 18:20:23 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include <string>
#include <vector>

class LocationConfig;

class ServerConfig {
   public:
	ServerConfig(const std::string& directive);
	ServerConfig(const ServerConfig& src);
	~ServerConfig();

	ServerConfig& operator=(ServerConfig const& rhs);

	const std::string& getDirective(const std::string& interface,
									const std::string& location,
									const std::string& name);

   private:
	ServerConfig();
	std::map<std::string, std::string> _directive;
	std::vector<LocationConfig*> _location;
};

#endif
