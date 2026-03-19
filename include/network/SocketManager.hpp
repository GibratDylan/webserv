#pragma once

#include <map>

#include "../config/GlobalConfig.hpp"
#include "TcpSocket.hpp"

class ServerConfig;

class SocketManager {
   public:
	SocketManager();
	~SocketManager();

	void setupSockets(const GlobalConfig& config);

	bool isListenSocket(int fd) const;
	ServerConfig* getServerConfig(int fd) const;

	TcpSocket* acceptConnection(int listenFd);

	const std::map<int, TcpSocket*>& getListenSockets() const;

   private:
	SocketManager(const SocketManager&);
	SocketManager& operator=(const SocketManager&);

	void clear();

	std::map<int, TcpSocket*> _listenSockets;
	std::map<int, ServerConfig*> _socketConfigs;
};
