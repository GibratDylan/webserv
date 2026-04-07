/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/07 00:00:00 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include <string>
#include <vector>

#include "IRequestHandler.hpp"

class Config;
class HttpRequest;
class HttpResponseBuilder;

class RequestProcessor {
   public:
	struct Result {
		enum Action { SEND_RESPONSE, START_CGI };

		Action action;
		HttpResponse response;
		std::string cgiRequestPath;
		std::string cgiExtension;

		Result()
			: action(SEND_RESPONSE),
			  response(),
			  cgiRequestPath(),
			  cgiExtension() {}
	};

	explicit RequestProcessor(const HttpResponseBuilder& responseBuilder);
	~RequestProcessor();

	Result process(const HttpRequest& request, const Config& serverConfig,
				   const Config& resolvedConfig) const;

   private:
	RequestProcessor();
	RequestProcessor(const RequestProcessor& other);
	RequestProcessor& operator=(const RequestProcessor& other);

	void resolveCgiTarget(const HttpRequest& request,
						  const Config& resolvedConfig,
						  std::string& cgiRequestPath,
						  std::string& cgiExtension) const;

	const HttpResponseBuilder& _responseBuilder;
	std::vector<IRequestHandler*> _handlers;
};

#endif
