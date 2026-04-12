/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveRegistry.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:11 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVEREGISTRY_HPP
#define DIRECTIVEREGISTRY_HPP

#include <map>
#include <string>
#include <vector>

#include "IDirectiveHandler.hpp"

class Config;

class DirectiveRegistry {
   public:
	enum Scope {
		SCOPE_GLOBAL,
		SCOPE_SERVER,
		SCOPE_LOCATION,
	};

	DirectiveRegistry();
	~DirectiveRegistry();

	bool has(Scope scope, const std::string& name) const;

	void apply(Scope scope, const std::string& name, Config& config,
			   const std::vector<std::string>& args) const;

   private:
	DirectiveRegistry(const DirectiveRegistry&);
	DirectiveRegistry& operator=(const DirectiveRegistry&);

	typedef std::map<std::string, const IDirectiveHandler*> HandlerMap;

	const IDirectiveHandler* getHandler(Scope scope,
										const std::string& name) const;
	void registerDirective(Scope scope, const std::string& name,
						   const IDirectiveHandler* handler);

	HandlerMap _globalHandlers;
	HandlerMap _serverHandlers;
	HandlerMap _locationHandlers;

	std::vector<const IDirectiveHandler*> _ownedHandlers;
};

#endif
