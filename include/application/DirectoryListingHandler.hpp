/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryListingHandler.hpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTORYLISTINGHANDLER_HPP
#define DIRECTORYLISTINGHANDLER_HPP

#include "IRequestHandler.hpp"

class DirectoryListingHandler : public IRequestHandler {
   public:
	DirectoryListingHandler();
	virtual ~DirectoryListingHandler();

	virtual bool canHandle(const HttpRequest& request,
						   const Config& resolvedConfig) const;
	virtual HttpResponse handle(const HttpRequest& request,
								const Config& resolvedConfig);
};

#endif
