/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/application/StaticFileHandler.hpp"

#include "../../include/config/Config.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/server/HttpRequest.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/PathUtils.hpp"

StaticFileHandler::StaticFileHandler() {}

StaticFileHandler::~StaticFileHandler() {}

bool StaticFileHandler::canHandle(const HttpRequest& request,
								  const Config& resolvedConfig) const {
	if (request.method != "GET") {
		return false;
	}

	std::string safe_path = PathUtils::normalizeForLocation(
		request.path, resolvedConfig.location_path);
	std::string root_path = addPath(resolvedConfig.root, safe_path);

	if (FileSystem::isDirectory(root_path)) {
		std::string index_path;
		std::string index_name;
		const bool has_index = FileSystem::findIndexFile(
			root_path, resolvedConfig.index, index_path, index_name);
		if (has_index) {
			return true;
		}
		return !resolvedConfig.autoindex;
	}

	return true;
}

HttpResponse StaticFileHandler::handle(const HttpRequest& request,
									   const Config& resolvedConfig) {
	std::string safe_path = PathUtils::normalizeForLocation(
		request.path, resolvedConfig.location_path);
	std::string root_path = addPath(resolvedConfig.root, safe_path);

	if (FileSystem::isDirectory(root_path)) {
		std::string index_path;
		std::string index_name;
		if (FileSystem::findIndexFile(root_path, resolvedConfig.index,
									  index_path, index_name)) {
			return HttpResponse::makeFileResponse(index_path, resolvedConfig);
		}

		return HttpResponse::makeErrorResponse(403, resolvedConfig);
	}

	return HttpResponse::makeFileResponse(root_path, resolvedConfig);
}
