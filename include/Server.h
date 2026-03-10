#pragma once

#include <poll.h>

#include <map>
#include <vector>

#include "SessionManager.h"
#include "config/GlobalConfig.hpp"
#include "exceptions.h"

class Connection;

class Server {
   private:
	std::vector<pollfd> _pollFds;

	std::map<int, Connection*> _connections;
	std::map<int, ServerConfig*> _listenSockets;

	std::map<int, Connection*> _cgiReadPipes;
	std::map<int, Connection*> _cgiWritePipes;

	SessionManager _sessionManager;

   private:
	void setupSockets();
	void addPollFd(int fd, short events);
	void removePollFd(int fd);
	void updatePollFd(int fd, short events);

	void acceptConnection(int listenFd);
	void removeConnection(int fd);
	void cleanupCgiPipes(Connection* conn);

	void handlePollEvents();
	void checkTimeouts();

   public:
	GlobalConfig config;
	Server(std::string& config_file_name);
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();

	void run();
   public: 	
	static int countPost;
	static int countGet;
	static int countConnections;

};
