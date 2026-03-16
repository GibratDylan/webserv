/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOMultiplexer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:36:02 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/16 08:57:10 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/network/IOMultiplexer.hpp"

#include <cstring>

IOMultiplexer::IOMultiplexer() {}

IOMultiplexer::~IOMultiplexer() {}

void IOMultiplexer::addFd(int fd, short events) {
	pollfd pfd;
	std::memset(&pfd, 0, sizeof(pfd));
	pfd.fd = fd;
	pfd.events = events;
	_pollFds.push_back(pfd);
}

void IOMultiplexer::modifyFd(int fd, short events) {
	for (size_t i = 0; i < _pollFds.size(); ++i) {
		if (_pollFds[i].fd == fd) {
			_pollFds[i].events = events;
			return;
		}
	}
}

void IOMultiplexer::removeFd(int fd) {
	for (size_t i = 0; i < _pollFds.size(); ++i) {
		if (_pollFds[i].fd == fd) {
			_pollFds.erase(_pollFds.begin() + static_cast<long>(i));
			return;
		}
	}
}

int IOMultiplexer::poll(int timeoutMs) {
	if (_pollFds.empty()) {
		return 0;
	}
	return ::poll(&_pollFds[0], _pollFds.size(), timeoutMs);
}

size_t IOMultiplexer::size() const {
	return _pollFds.size();
}

int IOMultiplexer::getFd(size_t index) const {
	return _pollFds[index].fd;
}

short IOMultiplexer::getRevents(size_t index) const {
	return _pollFds[index].revents;
}
