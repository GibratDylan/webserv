/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Copilot <copilot@openai.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 11:00:00 by Copilot           #+#    #+#             */
/*   Updated: 2026/04/02 11:00:00 by Copilot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include <sys/types.h>

#include <ctime>
#include <map>
#include <string>
#include <vector>

class Config;

class CgiExecutor {
   public:
	enum State { WRITING, READING, DONE };

   private:
	CgiExecutor();
	CgiExecutor(const CgiExecutor& src);
	CgiExecutor& operator=(const CgiExecutor& rhs);

	static std::vector<std::string> createEnv(
		const std::string& query, const std::string& method,
		const std::string& body,
		const std::map<std::string, std::string>& headers,
		const std::string& path, const std::string& uri, const Config* config);

   public:
	CgiExecutor(const std::string& path, const std::string& uri,
				const std::string& query, const std::string& method,
				const std::string& body,
				const std::map<std::string, std::string>& headers,
				const std::string& app, const Config* config);
	~CgiExecutor();

	bool run();
	void onRead();
	void onWrite();

	bool checkProcess();
	bool hasTimedOut() const;
	bool isDone() const;

	int getReadFd() const;
	int getWriteFd() const;
	int getExitStatus() const;
	State getState() const;

	const std::string& getOutput() const;
	bool hasOutput() const;

   private:
	int _fdFromCgi[2];
	int _fdToCgi[2];
	std::vector<std::string> _env;
	std::vector<std::string> _argv;

	pid_t _pid;
	time_t _startTime;
	int _exitStatus;

	State _state;
	std::string _readBuffer;
	std::string _writeBuffer;
};

#endif
