/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <ctime>

#include "../server/Connection.hpp"

class SessionManager;
class ServerConfig;
class CgiHandler;

class ClientConnection {
   public:
	ClientConnection(int fd, const ServerConfig& cfg,
					 SessionManager& sessionManager);
	~ClientConnection();

	int getFd() const;

	Connection::State getState() const;
	void setState(Connection::State state);

	void onRead();
	void onWrite();
	bool isDone() const;
	bool isTimeout(time_t now) const;

	CgiHandler* getCgi();
	const CgiHandler* getCgi() const;
	void finalizeCgi();

   private:
	ClientConnection(const ClientConnection&);
	ClientConnection& operator=(const ClientConnection&);

	Connection* _impl;
};

#endif
