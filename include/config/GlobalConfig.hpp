/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 16:07:06 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GLOBALCONFIG_HPP
#define GLOBALCONFIG_HPP

#include <string>
#include <vector>

#include "AConfig.hpp"

class ServerConfig;

class GlobalConfig : public AConfig {
   public:
	GlobalConfig(const std::string& config);
	GlobalConfig(const GlobalConfig& src);
	~GlobalConfig();

	GlobalConfig& operator=(const GlobalConfig& rhs);

	const std::string& getDirective(const std::string& interface,
									const std::string& location,
									const std::string& name);

   private:
	GlobalConfig();
	std::vector<ServerConfig*> _server;
};

#endif
