#include "Server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

Server::Server(int port) : _port(port) {
	setupSocket();
}

void Server::addFd(int fd) {
	pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	_fds.push_back(pfd);
}

void Server::setupSocket() {
	_fd = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	bind(_fd, (sockaddr*)&addr, sizeof(addr));
	listen(_fd, SOMAXCONN);

	fcntl(_fd, F_SETFL, O_NONBLOCK);
	addFd(_fd);
}

void Server::acceptClient() {
	int clientFd = accept(_fd, NULL, NULL);
	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	addFd(clientFd);
}

void Server::handleClient(int fd) {
	char buffer[1000];
	ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);

	if (bytes <= 0) {
		close(fd);
		_buffers.erase(fd);
		return;
	}

	_buffers[fd].append(buffer, bytes);

	if (_buffers[fd].find("\r\n\r\n") != std::string::npos) {
		std::string response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: 4\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"test";

		send(fd, response.c_str(), response.size(), 0);
		close(fd);
		_buffers.erase(fd);
	}
}

void Server::run() {
	while (true) {
		poll(&_fds[0], _fds.size(), -1);

		for (size_t i = 0; i < _fds.size(); ++i) {
			if (_fds[i].revents & POLLIN) {
				if (_fds[i].fd == _fd) {
					acceptClient();
				} else {
					handleClient(_fds[i].fd);
				}
			}
		}
	}
}
