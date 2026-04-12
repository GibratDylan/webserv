/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/07 18:11:44 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/application/UploadHandler.hpp"

#include "../../include/config/Config.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/server/HttpRequest.hpp"

UploadHandler::UploadHandler() {}

UploadHandler::~UploadHandler() {}

bool UploadHandler::canHandle(const HttpRequest& request,
							  const Config& resolvedConfig) const {
	(void)resolvedConfig;
	return request.method == "POST";
}

HttpResponse UploadHandler::handle(const HttpRequest& request,
								   const Config& resolvedConfig) {
	return HttpResponse::makePostResponse(request.path, request.body,
										  resolvedConfig);
}
