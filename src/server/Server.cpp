#include "../../include/server/Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <ctime>

#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/server/Connection.hpp"
#include "../../include/server/exceptions.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"
#include "../../include/utility/SignalSystem.hpp"

Server::Server(const std::string& config_file_name) : config(config_file_name) {
	if (HttpResponse::reasons.empty()) {
		HttpResponse::initReasons();
	}

	setupSockets();
	_sessionManager.setTtl(config.session_timeout);
}

Server::~Server() {
	for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); ++it) {
		delete it->second;
	}

	for (std::map<int, ServerConfig*>::iterator it = _listenSockets.begin(); it != _listenSockets.end(); ++it) {
		close(it->first);
	}
}

void Server::setupSockets() {
	for (std::map<int, ServerConfig>::iterator it = config.server.begin(); it != config.server.end(); ++it) {
		ServerConfig& srv = it->second;

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0) {
			throw SocketException("socket failed");
		}

		int opt = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			close(fd);
			throw SocketException("setsockopt failed");
		}

		sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(static_cast<unsigned short>(srv.port));
		addr.sin_addr.s_addr = srv.host.empty() ? INADDR_ANY : inet_addr(srv.host.c_str());

		if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
			close(fd);
			throw SocketException(std::string("bind to ") + srv.host + ":" + toString(srv.port) + " failed");
		}
		if (listen(fd, SOMAXCONN) < 0) {
			close(fd);
			throw SocketException("listen failed");
		}

		fcntl(fd, F_SETFL, O_NONBLOCK);

		_listenSockets[fd] = &srv;
		_multiplexer.addFd(fd, POLLIN);

		Logger::info(std::string(" Server listening on ") + srv.host + ":" + toString(srv.port));
	}
}

void Server::acceptConnection(int listenFd) {
	ServerConfig* cfg = _listenSockets[listenFd];

	if (_connections.size() >= cfg->max_connections) {
		int clientFd = accept(listenFd, NULL, NULL);
		if (clientFd >= 0) {
			close(clientFd);
		}
		Logger::warning(std::string(" Max connections reached on fd=") + toString(listenFd));
		return;
	}

	int clientFd = accept(listenFd, NULL, NULL);
	if (clientFd < 0) {
		Logger::error(std::string(" accept failed "));
		return;
	}

	fcntl(clientFd, F_SETFL, O_NONBLOCK);

	Connection* conn = new Connection(clientFd, *cfg, _sessionManager);
	_connections[clientFd] = conn;
	Logger::debug(std::string(" Accepted connection fd=") + toString(clientFd));

	_multiplexer.addFd(clientFd, POLLIN);
}

void Server::removeConnection(int fd) {
	std::map<int, Connection*>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		Connection* conn = it->second;
		cleanupCgiPipes(*conn);
		_multiplexer.removeFd(fd);
		Logger::debug(std::string(" Removing connection fd=") + toString(fd));

		delete conn;
		_connections.erase(it);
		return;
	}

	_multiplexer.removeFd(fd);
}

void Server::cleanupCgiPipes(const Connection& conn) {
	if (!conn.cgi) {
		return;
	}

	int readFd = conn.cgi->getCgiReadFd();
	int writeFd = conn.cgi->getCgiWriteFd();

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

		if (_listenSockets.find(fd) != _listenSockets.end()) {
			if (revents & POLLIN) {
				acceptConnection(fd);
			}
		} else if (_cgiWritePipes.find(fd) != _cgiWritePipes.end()) {
			Connection* connPtr = _cgiWritePipes[fd];

			if (revents & POLLOUT) {
				connPtr->cgi->onWriteCgi();
			}

			if (connPtr->cgi->getState() != CgiHandler::WRITING) {
				_multiplexer.removeFd(fd);
				_cgiWritePipes.erase(fd);
			}
		} else if (_cgiReadPipes.find(fd) != _cgiReadPipes.end()) {
			Connection& conn_ref = *(_cgiReadPipes.find(fd)->second);

			if (revents & (POLLIN | POLLHUP | POLLERR)) {
				conn_ref.cgi->onReadCgi();
			}

			if (conn_ref.cgi->isDone()) {
				conn_ref.finalizeCgi();
				cleanupCgiPipes(conn_ref);
				for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); ++it) {
					if (it->second == &conn_ref) {
						_multiplexer.modifyFd(it->first, POLLOUT);
						break;
					}
				}
			}
		} else {
			std::map<int, Connection*>::iterator connIt = _connections.find(fd);
			if (connIt == _connections.end()) {
				continue;
			}
			Connection& conn_ref = *(connIt->second);

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
				_cgiReadPipes[conn_ref.cgi->getCgiReadFd()] = &conn_ref;
				_multiplexer.addFd(conn_ref.cgi->getCgiReadFd(), POLLIN);
				_cgiWritePipes[conn_ref.cgi->getCgiWriteFd()] = &conn_ref;
				_multiplexer.addFd(conn_ref.cgi->getCgiWriteFd(), POLLOUT);
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

	for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); ++it) {
		Connection& conn_ref = *(it->second);

		if (conn_ref.cgi) {
			if (conn_ref.cgi->hasTimedOut()) {
				conn_ref.finalizeCgi();
				cleanupCgiPipes(conn_ref);
				_multiplexer.modifyFd(it->first, POLLOUT);
				continue;
			}

			if (conn_ref.cgi->getState() == CgiHandler::READING && !conn_ref.cgi->checkProcess()) {
				conn_ref.cgi->onReadCgi();
				if (conn_ref.cgi->isDone()) {
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
