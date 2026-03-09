#include "../../include/Server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <ctime>
#include <iostream>

#include "../../include/Connection.h"
#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/utils.h"
#include "cerrno"



Server::Server(std::string& config_file_name) : config(config_file_name) {
	if (HttpResponse::reasons.empty()) HttpResponse::initReasons();

	setupSockets();
	_sessionManager.setTtl(config.session_timeout);
}

Server::~Server() {
	for (std::map<int, Connection*>::iterator it = _connections.begin();
		 it != _connections.end(); ++it)
		delete it->second;

	for (std::map<int, ServerConfig*>::iterator it = _listenSockets.begin();
		 it != _listenSockets.end(); ++it)
		close(it->first);
}

void Server::setupSockets() {
	for (std::map<int, ServerConfig*>::iterator it = config.server.begin();
		 it != config.server.end(); ++it) {
		ServerConfig* srv = it->second;

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0) throw SocketException("socket failed");

		int opt = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(srv->port);
		// addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_addr.s_addr =
			srv->host.empty() ? INADDR_ANY : inet_addr(srv->host.c_str());

		if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
			throw SocketException(std::string("bind to ") + srv->host + ":" +
								  toString(srv->port) +
								  " failed: " + strerror(errno));
		if (listen(fd, SOMAXCONN) < 0)
			throw SocketException(std::string("listen failed: ") +
								  strerror(errno));

		fcntl(fd, F_SETFL, O_NONBLOCK);

		_listenSockets[fd] = srv;
		addPollFd(fd, POLLIN);

		std::cout << "Server listening on " << srv->host << ":" << srv->port
				  << std::endl;
	}
}

void Server::addPollFd(int fd, short events) {
	pollfd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	_pollFds.push_back(pfd);
}

void Server::removePollFd(int fd) {
	for (size_t i = 0; i < _pollFds.size(); ++i) {
		if (_pollFds[i].fd == fd) {
			_pollFds.erase(_pollFds.begin() + i);
			break;
		}
	}
}

void Server::updatePollFd(int fd, short events) {
	for (size_t i = 0; i < _pollFds.size(); ++i) {
		if (_pollFds[i].fd == fd) {
			_pollFds[i].events = events;
			break;
		}
	}
}

void Server::acceptConnection(int listenFd) {
	ServerConfig* config = _listenSockets[listenFd];

	if (_connections.size() >= config->max_connections) {
		int clientFd = accept(listenFd, NULL, NULL);
		if (clientFd >= 0) close(clientFd);
		return;
	}

	int clientFd = accept(listenFd, NULL, NULL);
	if (clientFd < 0) throw SocketException("accept failed");

	fcntl(clientFd, F_SETFL, O_NONBLOCK);

	_connections[clientFd] = new Connection(clientFd, config, &_sessionManager);

	addPollFd(clientFd, POLLIN);

}

void Server::removeConnection(int fd) {
	removePollFd(fd);

	std::map<int, Connection*>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		delete it->second;
		_connections.erase(it);
	}
}

void Server::cleanupCgiPipes(Connection* conn) {
	if (!conn || !conn->_cgi) {
		return;
	}

	int readFd = conn->_cgi->getCgiReadFd();
	int writeFd = conn->_cgi->getCgiWriteFd();

	_cgiReadPipes.erase(readFd);
	_cgiWritePipes.erase(writeFd);

	removePollFd(readFd);
	removePollFd(writeFd);
}

void Server::handlePollEvents() {
	for (size_t i = 0; i < _pollFds.size(); ++i) {
		int fd = _pollFds[i].fd;
		short revents = _pollFds[i].revents;

		if (revents == 0) continue;

		if (_listenSockets.find(fd) != _listenSockets.end()) {
			if (revents & POLLIN)
				acceptConnection(_listenSockets.find(fd)->first);
		} else if (_cgiWritePipes.find(fd) != _cgiWritePipes.end()) {
			Connection* conn = _cgiWritePipes.find(fd)->second;

			if (revents & POLLOUT) {
				conn->_cgi->onWriteCgi();
			}
		} else if (_cgiReadPipes.find(fd) != _cgiReadPipes.end()) {
			Connection* conn = _cgiReadPipes.find(fd)->second;

			if (revents & POLLIN) {
				conn->_cgi->onReadCgi();
			}

			if (conn->_cgi->isDone()) {
				conn->finalizeCgi();
				cleanupCgiPipes(conn);
				for (std::map<int, Connection*>::iterator it =
						 _connections.begin();
					 it != _connections.end(); ++it) {
					if (it->second == conn) {
						updatePollFd(it->first, POLLOUT);
						break;
					}
				}
			}
		} else {
			Connection* conn = _connections[fd];

			if (revents & POLLIN) conn->onRead();

			if (revents & POLLOUT) conn->onWrite();

			if (conn->getState() == Connection::WRITING)
				_pollFds[i].events = POLLOUT;
			else if (conn->getState() == Connection::INIT_CGI) {
				_cgiReadPipes[conn->_cgi->getCgiReadFd()] = conn;
				addPollFd(conn->_cgi->getCgiReadFd(), POLLIN);
				_cgiWritePipes[conn->_cgi->getCgiWriteFd()] = conn;
				addPollFd(conn->_cgi->getCgiWriteFd(), POLLOUT);
				_pollFds[i].events = 0;
				conn->setState(Connection::PROCESSING_CGI);
			} else
				_pollFds[i].events = POLLIN;

			if (conn->isDone()) removeConnection(fd);
		}
	}
}

void Server::checkTimeouts() {
	_sessionManager.cleanup();

	time_t now = std::time(NULL);

	std::vector<int> toRemove;

	for (std::map<int, Connection*>::iterator it = _connections.begin();
		 it != _connections.end(); ++it) {
		Connection* conn = it->second;

		if (conn->_cgi) {
			if (conn->_cgi->hasTimedOut()) {
				conn->finalizeCgi();
				cleanupCgiPipes(conn);
				updatePollFd(it->first, POLLOUT);
				continue;
			}

			if (conn->_cgi->getState() == CgiHandler::READING &&
				!conn->_cgi->checkProcess()) {
				conn->_cgi->onReadCgi();
				if (conn->_cgi->isDone()) {
					conn->finalizeCgi();
					cleanupCgiPipes(conn);
					updatePollFd(it->first, POLLOUT);
					continue;
				}
			}
		}

		if (conn->isTimeout(now)) {
			toRemove.push_back(it->first);
		}
	}

	for (size_t i = 0; i < toRemove.size(); ++i) removeConnection(toRemove[i]);
}

void Server::run() {
	while (true) {
		int ret = poll(&_pollFds[0], _pollFds.size(), 1000);
		if (ret < 0) continue;

		handlePollEvents();
		checkTimeouts();
	}
}
