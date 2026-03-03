/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:16:27 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 16:07:10 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>

#include "AConfig.hpp"

class LocationConfig : public AConfig {
   public:
	LocationConfig(const std::string& config);
	LocationConfig(const LocationConfig& src);
	~LocationConfig();

	LocationConfig& operator=(LocationConfig const& rhs);

	const std::string& getDirective(const std::string& interface,
									const std::string& location,
									const std::string& name);

   private:
	LocationConfig();
};

#endif
