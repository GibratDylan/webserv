/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>

#include "../config/GlobalConfig.hpp"
#include "../network/ClientConnection.hpp"
#include "../network/IOMultiplexer.hpp"
#include "../network/SocketManager.hpp"
#include "SessionManager.hpp"

class Server {
   private:
	IOMultiplexer _multiplexer;
	SocketManager _socketManager;

	std::map<int, ClientConnection*> _connections;

	std::map<int, ClientConnection*> _cgiReadPipes;
	std::map<int, ClientConnection*> _cgiWritePipes;

	SessionManager _sessionManager;

   private:
	void setupSockets();

	void acceptConnection(int listenFd);
	void removeConnection(int fd);
	void cleanupCgiPipes(ClientConnection& conn);

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

#endif
