/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/07 18:06:23 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include <string>

#include "../application/RequestProcessor.hpp"
#include "../http/HttpParser.hpp"
#include "../http/HttpResponse.hpp"
#include "../http/HttpResponseBuilder.hpp"
#include "../http/HttpRouter.hpp"
#include "HttpRequest.hpp"

class SessionManager;
struct Session;
class CgiHandler;
class ServerConfig;

class Connection {
   public:
	enum State { READING, PROCESSING, WRITING, DONE, PROCESSING_CGI, INIT_CGI };

   private:
	int _fd;
	State _state;

	std::string _readBuffer;
	std::string _writeBuffer;

	HttpRequest _request;
	HttpResponse _response;
	HttpParser _parser;
	HttpRouter _router;
	HttpResponseBuilder _responseBuilder;
	RequestProcessor _processor;

	time_t _lastActivity;

   private:
	void readFromSocket();
	void processRequest();
	bool tryStartCgi(const Config& resolvedConfig,
					 const std::string& cgiRequestPath,
					 const std::string& cgiExtension);
	void reset();

   public:
	Connection(int fd, const ServerConfig& cfg, SessionManager& sessionManager);
	~Connection();

	State getState() const;
	void setState(State state);
	int getFd() const;
	void onRead();
	void onWrite();
	bool isDone() const;
	bool isTimeout(time_t now) const;
	void handleSession();
	void finalizeCgi();

   public:
	const ServerConfig& config;
	SessionManager& sessionManager;
	CgiHandler* cgi;
};

#endif
