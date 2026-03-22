/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "IRequestHandler.hpp"

class DeleteHandler : public IRequestHandler {
   public:
	DeleteHandler();
	virtual ~DeleteHandler();

	virtual bool canHandle(const HttpRequest& request,
						   const Config& resolvedConfig) const;
	virtual HttpResponse handle(const HttpRequest& request,
								const Config& resolvedConfig);
};

#endif
