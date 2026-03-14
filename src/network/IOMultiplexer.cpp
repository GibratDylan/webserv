/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOMultiplexer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:36:02 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/14 13:21:10 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/network/IOMultiplexer.hpp"

IOMultiplexer::IOMultiplexer() {}

IOMultiplexer::~IOMultiplexer() {}

void IOMultiplexer::addFd(int fd, short events) {
	pollfd pfd = {};
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	_pollFds.push_back(pfd);
}

void IOMultiplexer::modifyFd(int fd, short events) {
	for (std::list<pollfd>::iterator fd_it = _pollFds.begin(); fd_it != _pollFds.end(); fd_it++) {
		if (fd_it->fd == fd) {
			fd_it->events = events;
		}
	}
}

void IOMultiplexer::removeFd(int fd) {
	for (std::list<pollfd>::iterator fd_it = _pollFds.begin(); fd_it != _pollFds.end(); fd_it++) {
		if (fd_it->fd == fd) {
			_pollFds.erase(fd_it);
		}
	}
}

short IOMultiplexer::getRevents(int fd) const {
	for (std::list<pollfd>::const_iterator fd_it = _pollFds.begin(); fd_it != _pollFds.end(); fd_it++) {
		if (fd_it->fd == fd) {
			return fd_it->revents;
		}
	}
	return 0;
}

/* ************************************************************************** */
