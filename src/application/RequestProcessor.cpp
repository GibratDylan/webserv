/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/07 00:00:00 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/application/RequestProcessor.hpp"

#include "../../include/application/DeleteHandler.hpp"
#include "../../include/application/DirectoryListingHandler.hpp"
#include "../../include/application/StaticFileHandler.hpp"
#include "../../include/application/UploadHandler.hpp"
#include "../../include/config/Config.hpp"
#include "../../include/http/HttpResponseBuilder.hpp"
#include "../../include/server/HttpRequest.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/PathUtils.hpp"

RequestProcessor::RequestProcessor(const HttpResponseBuilder& responseBuilder)
	: _responseBuilder(responseBuilder), _handlers() {
	_handlers.push_back(new DirectoryListingHandler());
	_handlers.push_back(new StaticFileHandler());
	_handlers.push_back(new UploadHandler());
	_handlers.push_back(new DeleteHandler());
}

RequestProcessor::~RequestProcessor() {
	for (std::vector<IRequestHandler*>::iterator it = _handlers.begin();
		 it != _handlers.end(); ++it) {
		delete *it;
	}
}

void RequestProcessor::resolveCgiTarget(const HttpRequest& request,
										const Config& resolvedConfig,
										std::string& cgiRequestPath,
										std::string& cgiExtension) const {
	cgiRequestPath = request.path;
	cgiExtension = PathUtils::getExtension(request.path);

	if (!cgiExtension.empty()) {
		return;
	}

	std::string safePath = PathUtils::normalizeForLocation(
		request.path, resolvedConfig.location_path);

	std::string rootPath = addPath(resolvedConfig.root, safePath);
	if (!FileSystem::isDirectory(rootPath)) {
		return;
	}

	std::string indexFilePath;
	std::string indexName;
	if (!FileSystem::findIndexFile(rootPath, resolvedConfig.index,
								   indexFilePath, indexName)) {
		return;
	}

	std::string indexExt = PathUtils::getExtension(indexFilePath);
	if (!resolvedConfig.cgi_handlers.count(indexExt)) {
		return;
	}

	cgiExtension = indexExt;
	if (!cgiRequestPath.empty() &&
		cgiRequestPath[cgiRequestPath.size() - 1] == '/') {
		cgiRequestPath += indexName;
	} else {
		cgiRequestPath += "/" + indexName;
	}
}

RequestProcessor::Result RequestProcessor::process(
	const HttpRequest& request, const Config& serverConfig,
	const Config& resolvedConfig) const {
	Result result;

	if (resolvedConfig.redirection.first != 0) {
		result.response =
			_responseBuilder.makeRedirect(resolvedConfig.redirection.first,
										  resolvedConfig.redirection.second);
		return result;
	}

	resolveCgiTarget(request, resolvedConfig, result.cgiRequestPath,
					 result.cgiExtension);

	if (!result.cgiExtension.empty() &&
		resolvedConfig.cgi_handlers.count(result.cgiExtension)) {
		result.action = Result::START_CGI;
		return result;
	}

	for (std::vector<IRequestHandler*>::const_iterator it = _handlers.begin();
		 it != _handlers.end(); ++it) {
		if ((*it)->canHandle(request, resolvedConfig)) {
			result.response = (*it)->handle(request, resolvedConfig);
			return result;
		}
	}

	result.response = _responseBuilder.makeError(405, serverConfig);
	return result;
}
