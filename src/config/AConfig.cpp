/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AConfig.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:34:54 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 16:15:46 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

AConfig::AConfig() {}

AConfig::AConfig(const AConfig& src) : _config(src._config) {}

AConfig::~AConfig() {}

AConfig& AConfig::operator=(AConfig const& rhs) {
	if (this != &rhs) {
		_config = rhs._config;
	}
	return *this;
}

const std::map<std::string, std::string>& AConfig::getConfig() const {
	return _config;
}

void AConfig::setConfig(const std::map<std::string, std::string>& newConfig) {
	_config = newConfig;
}
