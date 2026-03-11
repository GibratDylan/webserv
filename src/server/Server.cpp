#include "../../include/Server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <ctime>
#include <iostream>

#include "../../include/Connection.h"
#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utils.h"
#include "cerrno"

int Server::countPost = 0;
int Server::countGet = 0;
int Server::countConnections = 0;

Server::Server(const std::string& config_file_name) : config(config_file_name) {
	if (HttpResponse::reasons.empty()) {
		HttpResponse::initReasons();
	}

	setupSockets();
	_sessionManager.setTtl(config.session_timeout);
}

Server::~Server() {
	for (size_t i = 0; i < _connectionPtrs.size(); ++i) {
		delete _connectionPtrs[i];
	}

	for (std::map<int, ServerConfig*>::iterator it = _listenSockets.begin(); it != _listenSockets.end(); ++it) {
		close(it->first);
	}
}

void Server::setupSockets() {
	for (std::map<int, ServerConfig>::iterator it = config.server.begin(); it != config.server.end(); ++it) {
		ServerConfig& srv = it->second;

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0) throw SocketException("socket failed");

		int opt = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(srv.port);
		// addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_addr.s_addr = srv.host.empty() ? INADDR_ANY : inet_addr(srv.host.c_str());

		if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
			throw SocketException(std::string("bind to ") + srv.host + ":" + toString(srv.port) + " failed: " + strerror(errno));
		if (listen(fd, SOMAXCONN) < 0) throw SocketException(std::string("listen failed: ") + strerror(errno));

		fcntl(fd, F_SETFL, O_NONBLOCK);

		_listenSockets[fd] = &srv;
		addPollFd(fd, POLLIN);

		Logger::info(std::string(" Server listening on ") + srv.host + ":" + toString(srv.port));
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
	ServerConfig* cfg = _listenSockets[listenFd];

	if (_connections.size() >= (size_t)cfg->max_connections) {
		int clientFd = accept(listenFd, NULL, NULL);
		if (clientFd >= 0) close(clientFd);
		Logger::warning(std::string(" Max connections reached on fd=") + toString(listenFd));
		return;
	}

	int clientFd = accept(listenFd, NULL, NULL);
	if (clientFd < 0) throw SocketException("accept failed");

	fcntl(clientFd, F_SETFL, O_NONBLOCK);

	Connection* conn = new Connection(clientFd, *cfg, _sessionManager);
	_connectionPtrs.push_back(conn);
	_connections[clientFd] = conn;
	Logger::debug(std::string(" Accepted connection fd=") + toString(clientFd));

	addPollFd(clientFd, POLLIN);
}

void Server::removeConnection(int fd) {
	std::map<int, Connection*>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		const Connection& conn = *(it->second);
		cleanupCgiPipes(conn);
		removePollFd(fd);
		Logger::debug(std::string(" Removing connection fd=") + toString(fd));

		// Find and delete the connection from _connectionPtrs
		for (std::vector<Connection*>::iterator ptrIt = _connectionPtrs.begin(); ptrIt != _connectionPtrs.end(); ++ptrIt) {
			if (*ptrIt == &conn) {
				delete *ptrIt;
				_connectionPtrs.erase(ptrIt);
				break;
			}
		}
		_connections.erase(it);
		Server::countConnections--;
		return;
	}

	removePollFd(fd);
}

void Server::cleanupCgiPipes(const Connection& conn) {
	if (!conn.cgi) {
		return;
	}

	int readFd = conn.cgi->getCgiReadFd();
	int writeFd = conn.cgi->getCgiWriteFd();

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
			if (revents & POLLIN) acceptConnection(_listenSockets.find(fd)->first);
		} else if (_cgiWritePipes.find(fd) != _cgiWritePipes.end()) {
			Connection& conn = *(_cgiWritePipes.find(fd)->second);

			if (revents & POLLOUT) {
				conn.cgi->onWriteCgi();
			}
		} else if (_cgiReadPipes.find(fd) != _cgiReadPipes.end()) {
			Connection& conn = *(_cgiReadPipes.find(fd)->second);

			if (revents & (POLLIN | POLLHUP | POLLERR)) {
				conn.cgi->onReadCgi();
			}

			if (conn.cgi->isDone()) {
				conn.finalizeCgi();
				cleanupCgiPipes(conn);
				for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); ++it) {
					if (it->second == &conn) {
						updatePollFd(it->first, POLLOUT);
						break;
					}
				}
			}
		} else {
			std::map<int, Connection*>::iterator connIt = _connections.find(fd);
			if (connIt == _connections.end()) {
				continue;
			}
			Connection& conn = *(connIt->second);

			if (revents & POLLIN) conn.onRead();

			if (revents & POLLOUT) conn.onWrite();

			if (conn.getState() == Connection::WRITING)
				updatePollFd(fd, POLLOUT);
			else if (conn.getState() == Connection::INIT_CGI) {
				_cgiReadPipes[conn.cgi->getCgiReadFd()] = &conn;
				addPollFd(conn.cgi->getCgiReadFd(), POLLIN);
				_cgiWritePipes[conn.cgi->getCgiWriteFd()] = &conn;
				addPollFd(conn.cgi->getCgiWriteFd(), POLLOUT);
				// _pollFds[i].events = 0;
				conn.setState(Connection::PROCESSING_CGI);
			} else
				updatePollFd(fd, POLLIN);

			if (conn.isDone()) {
				removeConnection(fd);
			}
		}
	}
}

void Server::checkTimeouts() {
	_sessionManager.cleanup();

	time_t now = std::time(NULL);

	std::vector<int> toRemove;

	for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); ++it) {
		Connection& conn = *(it->second);

		if (conn.cgi) {
			if (conn.cgi->hasTimedOut()) {
				conn.finalizeCgi();
				cleanupCgiPipes(conn);
				updatePollFd(it->first, POLLOUT);
				continue;
			}

			if (conn.cgi->getState() == CgiHandler::READING && !conn.cgi->checkProcess()) {
				conn.cgi->onReadCgi();
				if (conn.cgi->isDone()) {
					conn.finalizeCgi();
					cleanupCgiPipes(conn);
					updatePollFd(it->first, POLLOUT);
					continue;
				}
			}
		}

		if (conn.isTimeout(now)) {
			toRemove.push_back(it->first);
		}
	}

	for (size_t i = 0; i < toRemove.size(); ++i) removeConnection(toRemove[i]);
}

void Server::run() {
	while (true) {
		int ret = poll(&_pollFds[0], _pollFds.size(), 1000);
		if (ret < 0) {
			continue;
		}

		handlePollEvents();
		checkTimeouts();
	}
}
