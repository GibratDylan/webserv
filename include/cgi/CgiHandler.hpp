/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 14:27:54 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/02 13:25:13 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <sys/types.h>

#include <ctime>
#include <map>
#include <string>

#include "CgiExecutor.hpp"
#include "CgiResponseParser.hpp"

class Config;
class HttpResponse;

class CgiHandler {
   public:
	enum State { READING, PROCESSING, WRITING, DONE };

   private:
	CgiHandler& operator=(const CgiHandler& rhs);
	CgiHandler(const CgiHandler& src);
	CgiHandler();
	void markDone();

   public:
	CgiHandler(const std::string& path, const std::string& uri,
			   const std::string& query, const std::string& method,
			   const std::string& body,
			   const std::map<std::string, std::string>& headers,
			   const std::string& app, const Config* config);

	~CgiHandler();

	bool run();

	int getCgiReadFd() const;
	int getCgiWriteFd() const;
	void onReadCgi();
	void onWriteCgi();

	bool isDone() const;
	State getState() const;
	int getCode() const;
	bool hasTimedOut() const;
	bool checkProcess();

	void parseResponse();
	HttpResponse buildResponse() const;

   private:
	CgiExecutor _executor;
	CgiResponseParser _parser;
	State _state;

	bool _headersParsed;
	std::map<std::string, std::string> _responseHeaders;

   public:
	int code;
	std::string type;
	std::string body;
};

#endif
