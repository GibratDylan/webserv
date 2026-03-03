/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:47:39 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 16:07:13 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>

#include "AConfig.hpp"

class LocationConfig;

class ServerConfig : public AConfig {
   public:
	ServerConfig(const std::string& config);
	ServerConfig(const ServerConfig& src);
	~ServerConfig();

	ServerConfig& operator=(ServerConfig const& rhs);

	const std::string& getDirective(const std::string& interface,
									const std::string& location,
									const std::string& name);

   private:
	ServerConfig();
	std::vector<LocationConfig*> _location;
};

#endif
