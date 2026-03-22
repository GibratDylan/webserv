/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATICFILEHANDLER_HPP
#define STATICFILEHANDLER_HPP

#include "IRequestHandler.hpp"

class StaticFileHandler : public IRequestHandler {
   public:
	StaticFileHandler();
	virtual ~StaticFileHandler();

	virtual bool canHandle(const HttpRequest& request,
						   const Config& resolvedConfig) const;
	virtual HttpResponse handle(const HttpRequest& request,
								const Config& resolvedConfig);
};

#endif
