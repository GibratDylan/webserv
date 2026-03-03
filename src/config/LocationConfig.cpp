/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:16:27 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/03 15:35:36 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

LocationConfig::LocationConfig() {}

LocationConfig::LocationConfig(const LocationConfig& src) {}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

LocationConfig::~LocationConfig() {}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

LocationConfig& LocationConfig::operator=(LocationConfig const& rhs) {
	// if ( this != &rhs )
	//{
	// this->_value = rhs.getValue();
	//}
	return *this;
}

std::ostream& operator<<(std::ostream& o, LocationConfig const& i) {
	// o << "Value = " << i.getValue();
	return o;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

/* ************************************************************************** */
