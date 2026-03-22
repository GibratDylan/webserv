/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:21 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include <ctime>
#include <map>
#include <string>

class HttpRequest;
class HttpResponse;

struct Session {
	std::string id;
	std::map<std::string, std::string> data;
	time_t last_access;
};

class SessionManager {
   private:
	std::map<std::string, Session> _sessions;
	size_t _ttl;

   public:
	SessionManager();

	std::string createSession();
	Session* getSession(const std::string& id);
	void cleanup();
	void setTtl(size_t ttl);
	void transferSession(HttpRequest* request, HttpResponse* response);
};

#endif
