/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:32:34 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 16:18:15 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

GlobalConfig::GlobalConfig() {}

GlobalConfig::GlobalConfig(const GlobalConfig& src)
	: AConfig(src), _server(src._server) {}

GlobalConfig::~GlobalConfig() {}

GlobalConfig& GlobalConfig::operator=(GlobalConfig const& rhs) {
	if (this != &rhs) {
		this->setConfig(rhs.getConfig());
	}
	return *this;
}
