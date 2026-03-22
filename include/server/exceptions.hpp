/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

class SocketException : public std::runtime_error {
   public:
	SocketException(const std::string& msg) : std::runtime_error(msg) {}
};

class ParsingException : public std::runtime_error {
   public:
	ParsingException(const std::string& msg) : std::runtime_error(msg) {}
};

class ExecveException : public std::exception {
   public:
	ExecveException() {}
};

#endif
