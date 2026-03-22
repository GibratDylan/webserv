/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/network/ClientConnection.hpp"

#include "../../include/cgi/CgiHandler.hpp"

ClientConnection::ClientConnection(int fd, const ServerConfig& cfg,
								   SessionManager& sessionManager)
	: _impl(new Connection(fd, cfg, sessionManager)) {}

ClientConnection::~ClientConnection() {
	delete _impl;
}

int ClientConnection::getFd() const {
	return _impl ? _impl->getFd() : -1;
}

Connection::State ClientConnection::getState() const {
	return _impl->getState();
}

void ClientConnection::setState(Connection::State state) {
	_impl->setState(state);
}

void ClientConnection::onRead() {
	_impl->onRead();
}

void ClientConnection::onWrite() {
	_impl->onWrite();
}

bool ClientConnection::isDone() const {
	return _impl->isDone();
}

bool ClientConnection::isTimeout(time_t now) const {
	return _impl->isTimeout(now);
}

CgiHandler* ClientConnection::getCgi() {
	return _impl->cgi;
}

const CgiHandler* ClientConnection::getCgi() const {
	return _impl->cgi;
}

void ClientConnection::finalizeCgi() {
	_impl->finalizeCgi();
}
