/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/07 18:11:41 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/application/DeleteHandler.hpp"

#include "../../include/config/Config.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/server/HttpRequest.hpp"

DeleteHandler::DeleteHandler() {}

DeleteHandler::~DeleteHandler() {}

bool DeleteHandler::canHandle(const HttpRequest& request,
							  const Config& resolvedConfig) const {
	(void)resolvedConfig;
	return request.method == "DELETE";
}

HttpResponse DeleteHandler::handle(const HttpRequest& request,
								   const Config& resolvedConfig) {
	return HttpResponse::makeDeleteResponse(request.path, resolvedConfig);
}
