#pragma once

#include <map>
#include <vector>

#include "../config/GlobalConfig.hpp"
#include "../network/IOMultiplexer.hpp"
#include "SessionManager.hpp"

class Connection;

class Server {
   private:
	IOMultiplexer _multiplexer;

	std::map<int, Connection*> _connections;
	std::map<int, ServerConfig*> _listenSockets;

	std::map<int, Connection*> _cgiReadPipes;
	std::map<int, Connection*> _cgiWritePipes;

	SessionManager _sessionManager;

   private:
	void setupSockets();

	void acceptConnection(int listenFd);
	void removeConnection(int fd);
	void cleanupCgiPipes(const Connection& conn);

	void handlePollEvents();
	void checkTimeouts();

   public:
	GlobalConfig config;
	Server(const std::string& config_file_name);
	~Server();

	void run();

   private:
	Server(const Server&);
	Server& operator=(const Server&);
};
