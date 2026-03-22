/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRequestHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include "../http/HttpResponse.hpp"

class Config;
class HttpRequest;

class IRequestHandler {
   public:
	virtual ~IRequestHandler() {}

	virtual bool canHandle(const HttpRequest& request,
						   const Config& resolvedConfig) const = 0;
	virtual HttpResponse handle(const HttpRequest& request,
								const Config& resolvedConfig) = 0;

   protected:
	IRequestHandler() {}

   private:
	IRequestHandler(const IRequestHandler& other);
	IRequestHandler& operator=(const IRequestHandler& other);
};

#endif
