/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UPLOADHANDLER_HPP
#define UPLOADHANDLER_HPP

#include "IRequestHandler.hpp"

class UploadHandler : public IRequestHandler {
   public:
	UploadHandler();
	virtual ~UploadHandler();

	virtual bool canHandle(const HttpRequest& request,
						   const Config& resolvedConfig) const;
	virtual HttpResponse handle(const HttpRequest& request,
								const Config& resolvedConfig);
};

#endif
