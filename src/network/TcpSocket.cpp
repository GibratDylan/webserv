/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpSocket.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/network/TcpSocket.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "../../include/server/exceptions.hpp"

TcpSocket::TcpSocket() : _fd(-1), _listening(false), _host(""), _port(0) {}

TcpSocket::TcpSocket(int fd)
	: _fd(fd), _listening(false), _host(""), _port(0) {}

TcpSocket::~TcpSocket() {
	close();
}

void TcpSocket::create() {
	if (_fd != -1) {
		return;
	}

	_fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0) {
		throw SocketException("socket failed");
	}
}

void TcpSocket::setReuseAddr(bool reuse) {
	if (_fd < 0) {
		throw SocketException("setsockopt on invalid socket");
	}

	int opt = reuse ? 1 : 0;
	if (::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw SocketException("setsockopt failed");
	}
}

void TcpSocket::bind(const std::string& host, int port) {
	if (_fd < 0) {
		throw SocketException("bind on invalid socket");
	}

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast<unsigned short>(port));
	addr.sin_addr.s_addr = host.empty() ? INADDR_ANY : inet_addr(host.c_str());

	if (::bind(_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		throw SocketException(std::string("bind failed: ") +
							  std::strerror(errno));
	}

	_host = host;
	_port = port;
}

void TcpSocket::listen(int backlog) {
	if (_fd < 0) {
		throw SocketException("listen on invalid socket");
	}

	if (::listen(_fd, backlog) < 0) {
		throw SocketException("listen failed");
	}
	_listening = true;
}

void TcpSocket::setNonBlocking(bool nonBlocking) {
	if (_fd < 0) {
		throw SocketException("fcntl on invalid socket");
	}

	int flags = ::fcntl(_fd, F_GETFL, 0);
	if (flags < 0) {
		throw SocketException("fcntl(F_GETFL) failed");
	}

	if (nonBlocking) {
		flags |= O_NONBLOCK;
	} else {
		flags &= ~O_NONBLOCK;
	}

	if (::fcntl(_fd, F_SETFL, flags) < 0) {
		throw SocketException("fcntl(F_SETFL) failed");
	}
}

TcpSocket* TcpSocket::accept() {
	if (_fd < 0) {
		throw SocketException("accept on invalid socket");
	}

	int clientFd = ::accept(_fd, NULL, NULL);
	if (clientFd < 0) {
		return NULL;
	}
	return new TcpSocket(clientFd);
}

ssize_t TcpSocket::send(const void* data, size_t len) {
	if (_fd < 0) {
		return -1;
	}
	return ::send(_fd, data, len, MSG_DONTWAIT | MSG_NOSIGNAL);
}

ssize_t TcpSocket::receive(void* buffer, size_t len) {
	if (_fd < 0) {
		return -1;
	}
	return ::recv(_fd, buffer, len, MSG_DONTWAIT);
}

int TcpSocket::getFd() const {
	return _fd;
}

std::string TcpSocket::getHost() const {
	return _host;
}

int TcpSocket::getPort() const {
	return _port;
}

bool TcpSocket::isListening() const {
	return _listening;
}

void TcpSocket::close() {
	if (_fd >= 0) {
		::close(_fd);
		_fd = -1;
	}
	_listening = false;
}

int TcpSocket::release() {
	int fd = _fd;
	_fd = -1;
	_listening = false;
	return fd;
}
