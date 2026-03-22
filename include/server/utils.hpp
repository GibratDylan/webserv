/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>

template <typename T>
std::string toString(const T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool isNumber(const std::string& str);
size_t conversionBytesParsing(const std::string& str);
std::string trim(const std::string& str);
std::string addPath(const std::string& base, const std::string& suffix);

#endif
