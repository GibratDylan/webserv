/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 14:27:54 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/07 20:44:40 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <sys/types.h>

#include <ctime>
#include <map>
#include <string>
#include <vector>

class Config;
class HttpResponse;

class CgiHandler {
   public:
	enum State { READING, PROCESSING, WRITING, DONE };

   private:
	CgiHandler& operator=(const CgiHandler& rhs);
	CgiHandler(const CgiHandler& src);
	CgiHandler();
	static std::vector<std::string> createEnv(
		const std::string& query, const std::string& method,
		const std::string& body,
		const std::map<std::string, std::string>& headers,
		const std::string& path, const std::string& root_path, Config* config);

   public:
	CgiHandler(const std::string& path, const std::string& query,
			   const std::string& method, const std::string& body,
			   const std::map<std::string, std::string>& headers,
			   const std::string& app, Config* config);

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
	int _fdFromCgi[2];
	int _fdToCgi[2];
	std::vector<std::string> _env;
	std::vector<std::string> _argv;
	std::string _method;

	pid_t _pid;
	time_t _startTime;
	int _exitStatus;

	State _state;
	std::string _readBuffer;
	std::string _writeBuffer;

	bool _headersParsed;
	std::map<std::string, std::string> _responseHeaders;

   public:
	int code;
	std::string type;
	std::string body;
};

#endif
