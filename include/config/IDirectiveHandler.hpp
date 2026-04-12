/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IDirectiveHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:11 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IDIRECTIVEHANDLER_HPP
#define IDIRECTIVEHANDLER_HPP

#include <string>
#include <vector>

class Config;

class IDirectiveHandler {
   public:
	virtual ~IDirectiveHandler() {}

	virtual void validate(const std::vector<std::string>& args) const = 0;
	virtual void apply(Config& config,
					   const std::vector<std::string>& args) const = 0;
};

#endif
