/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 21:46:51 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GLOBALCONFIG_HPP
#define GLOBALCONFIG_HPP

#include <map>
#include <string>
#include <vector>

class ServerConfig;

class GlobalConfig {
   public:
	GlobalConfig(const std::string& directive);
	GlobalConfig(const GlobalConfig& src);
	~GlobalConfig();

	GlobalConfig& operator=(const GlobalConfig& rhs);

	const std::string& getDirective(const std::string& interface,
									const std::string& location,
									const std::string& name);

	void printDirectives() const;

   private:
	std::map<std::string, std::vector<std::string> > _directive;
	std::vector<ServerConfig*> _server;
};

#endif
