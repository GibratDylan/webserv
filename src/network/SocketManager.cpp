/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/network/SocketManager.hpp"

#include <sys/socket.h>

#include "../../include/config/ServerConfig.hpp"

SocketManager::SocketManager() {}

SocketManager::~SocketManager() {
	clear();
}

void SocketManager::clear() {
	for (std::map<int, TcpSocket*>::iterator it = _listenSockets.begin();
		 it != _listenSockets.end(); ++it) {
		delete it->second;
	}
	_listenSockets.clear();
	_socketConfigs.clear();
}

void SocketManager::setupSockets(const GlobalConfig& config) {
	clear();

	try {
		for (std::map<int, ServerConfig>::const_iterator it =
				 config.server.begin();
			 it != config.server.end(); ++it) {
			const ServerConfig& srv = it->second;

			TcpSocket* sock = new TcpSocket();
			sock->create();
			sock->setReuseAddr(true);
			sock->bind(srv.host, srv.port);
			sock->listen(SOMAXCONN);
			sock->setNonBlocking(true);

			int fd = sock->getFd();
			_listenSockets[fd] = sock;
			_socketConfigs[fd] = const_cast<ServerConfig*>(&srv);
		}
	} catch (...) {
		clear();
		throw;
	}
}

bool SocketManager::isListenSocket(int fd) const {
	return _listenSockets.find(fd) != _listenSockets.end();
}

ServerConfig* SocketManager::getServerConfig(int fd) const {
	std::map<int, ServerConfig*>::const_iterator it = _socketConfigs.find(fd);
	if (it == _socketConfigs.end()) {
		return NULL;
	}
	return it->second;
}

TcpSocket* SocketManager::acceptConnection(int listenFd) {
	std::map<int, TcpSocket*>::iterator it = _listenSockets.find(listenFd);
	if (it == _listenSockets.end()) {
		return NULL;
	}

	TcpSocket* client = it->second->accept();
	if (!client) {
		return NULL;
	}

	try {
		client->setNonBlocking(true);
	} catch (...) {
		delete client;
		throw;
	}

	return client;
}

const std::map<int, TcpSocket*>& SocketManager::getListenSockets() const {
	return _listenSockets;
}
