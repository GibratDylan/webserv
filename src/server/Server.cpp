#include "../../include/server/Server.hpp"

#include <cstring>
#include <ctime>

#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/server/exceptions.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utility/SignalSystem.hpp"

Server::Server(const std::string& config_file_name) : config(config_file_name) {
	HttpResponse::initReasons();

	setupSockets();
	_sessionManager.setTtl(config.session_timeout);
}

Server::~Server() {
	for (std::map<int, ClientConnection*>::iterator it = _connections.begin();
		 it != _connections.end(); ++it) {
		delete it->second;
	}
}

void Server::setupSockets() {
	_socketManager.setupSockets(config);

	const std::map<int, TcpSocket*>& listenSockets =
		_socketManager.getListenSockets();
	for (std::map<int, TcpSocket*>::const_iterator it = listenSockets.begin();
		 it != listenSockets.end(); ++it) {
		TcpSocket* sock = it->second;
		int fd = it->first;
		_multiplexer.addFd(fd, POLLIN);
		Logger::info(std::string(" Server listening on ") + sock->getHost() +
					 ":" + toString(sock->getPort()));
	}
}

void Server::acceptConnection(int listenFd) {
	ServerConfig* cfg = _socketManager.getServerConfig(listenFd);
	if (!cfg) {
		return;
	}

	if (_connections.size() >= cfg->max_connections) {
		TcpSocket* clientSock = _socketManager.acceptConnection(listenFd);
		delete clientSock;
		Logger::warning(std::string(" Max connections reached on fd=") +
						toString(listenFd));
		return;
	}

	TcpSocket* clientSock = _socketManager.acceptConnection(listenFd);
	if (!clientSock) {
		Logger::error(std::string(" accept failed "));
		return;
	}
	int clientFd = clientSock->release();
	delete clientSock;

	ClientConnection* conn =
		new ClientConnection(clientFd, *cfg, _sessionManager);
	_connections[clientFd] = conn;
	Logger::debug(std::string(" Accepted connection fd=") + toString(clientFd));

	_multiplexer.addFd(clientFd, POLLIN);
}

void Server::removeConnection(int fd) {
	std::map<int, ClientConnection*>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		ClientConnection* conn = it->second;
		cleanupCgiPipes(*conn);
		_multiplexer.removeFd(fd);
		Logger::debug(std::string(" Removing connection fd=") + toString(fd));

		delete conn;
		_connections.erase(it);
		return;
	}

	_multiplexer.removeFd(fd);
}

void Server::cleanupCgiPipes(ClientConnection& conn) {
	CgiHandler* cgi = conn.getCgi();
	if (!cgi) {
		return;
	}

	int readFd = cgi->getCgiReadFd();
	int writeFd = cgi->getCgiWriteFd();

	_cgiReadPipes.erase(readFd);
	_cgiWritePipes.erase(writeFd);

	_multiplexer.removeFd(readFd);
	_multiplexer.removeFd(writeFd);
}

void Server::handlePollEvents() {
	std::vector<int> fds;
	std::vector<short> reventsVec;
	for (size_t i = 0; i < _multiplexer.size(); ++i) {
		fds.push_back(_multiplexer.getFd(i));
		reventsVec.push_back(_multiplexer.getRevents(i));
	}

	for (size_t i = 0; i < fds.size(); ++i) {
		int fd = fds[i];
		short revents = reventsVec[i];

		if (revents == 0) {
			continue;
		}

		if (_socketManager.isListenSocket(fd)) {
			if (revents & POLLIN) {
				acceptConnection(fd);
			}
		} else if (_cgiWritePipes.find(fd) != _cgiWritePipes.end()) {
			ClientConnection* connPtr = _cgiWritePipes[fd];

			if (revents & POLLOUT) {
				if (connPtr->getCgi()) {
					connPtr->getCgi()->onWriteCgi();
				}
			}

			if (!connPtr->getCgi() ||
				connPtr->getCgi()->getState() != CgiHandler::WRITING) {
				_multiplexer.removeFd(fd);
				_cgiWritePipes.erase(fd);
			}
		} else if (_cgiReadPipes.find(fd) != _cgiReadPipes.end()) {
			ClientConnection& conn_ref = *(_cgiReadPipes.find(fd)->second);

			if (revents & (POLLIN | POLLHUP | POLLERR)) {
				if (conn_ref.getCgi()) {
					conn_ref.getCgi()->onReadCgi();
				}
			}

			if (conn_ref.getCgi() && conn_ref.getCgi()->isDone()) {
				conn_ref.finalizeCgi();
				cleanupCgiPipes(conn_ref);
				_multiplexer.modifyFd(conn_ref.getFd(), POLLOUT);
			}
		} else {
			std::map<int, ClientConnection*>::iterator connIt =
				_connections.find(fd);
			if (connIt == _connections.end()) {
				continue;
			}
			ClientConnection& conn_ref = *(connIt->second);

			if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
				removeConnection(fd);
				continue;
			}

			if (revents & POLLIN) {
				conn_ref.onRead();
			}

			if (conn_ref.isDone()) {
				removeConnection(fd);
				continue;
			}

			if (revents & POLLOUT) {
				conn_ref.onWrite();
			}

			if (conn_ref.isDone()) {
				removeConnection(fd);
				continue;
			}

			if (conn_ref.getState() == Connection::WRITING) {
				_multiplexer.modifyFd(fd, POLLOUT);
			} else if (conn_ref.getState() == Connection::INIT_CGI) {
				if (conn_ref.getCgi()) {
					_cgiReadPipes[conn_ref.getCgi()->getCgiReadFd()] =
						&conn_ref;
					_multiplexer.addFd(conn_ref.getCgi()->getCgiReadFd(),
									   POLLIN);
					_cgiWritePipes[conn_ref.getCgi()->getCgiWriteFd()] =
						&conn_ref;
					_multiplexer.addFd(conn_ref.getCgi()->getCgiWriteFd(),
									   POLLOUT);
				}
				conn_ref.setState(Connection::PROCESSING_CGI);
			} else {
				_multiplexer.modifyFd(fd, POLLIN);
			}
		}
	}
}

void Server::checkTimeouts() {
	_sessionManager.cleanup();

	time_t now = std::time(NULL);

	std::vector<int> toRemove;

	for (std::map<int, ClientConnection*>::iterator it = _connections.begin();
		 it != _connections.end(); ++it) {
		ClientConnection& conn_ref = *(it->second);

		if (conn_ref.getCgi()) {
			if (conn_ref.getCgi()->hasTimedOut()) {
				conn_ref.finalizeCgi();
				cleanupCgiPipes(conn_ref);
				_multiplexer.modifyFd(it->first, POLLOUT);
				continue;
			}

			if (conn_ref.getCgi()->getState() == CgiHandler::READING &&
				!conn_ref.getCgi()->checkProcess()) {
				conn_ref.getCgi()->onReadCgi();
				if (conn_ref.getCgi() && conn_ref.getCgi()->isDone()) {
					conn_ref.finalizeCgi();
					cleanupCgiPipes(conn_ref);
					_multiplexer.modifyFd(it->first, POLLOUT);
					continue;
				}
			}
		}

		if (conn_ref.isTimeout(now)) {
			toRemove.push_back(it->first);
		}
	}

	for (size_t i = 0; i < toRemove.size(); ++i) {
		removeConnection(toRemove[i]);
	}
}

void Server::run() {
	while (SignalSystem::running == 1) {
		int ret = _multiplexer.poll(5);
		if (ret < 0) {
			continue;
		}

		handlePollEvents();
		checkTimeouts();
	}
}
