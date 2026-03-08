#include <sstream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include "SessionManager.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "utils.h"

std::string generateSessionId()
{
	std::ostringstream ss;
	srand(std::time(NULL) + rand());
	ss << std::time(NULL) << rand() << std::clock();
	return ss.str();
}

std::string extractSessionId(const std::string& cookies) {
	std::string sessionKey = "session_id=";
	size_t pos = cookies.find(sessionKey);
	if (pos == std::string::npos)
		return "";

	pos += sessionKey.size();
	size_t end = cookies.find(';', pos);
	if (end == std::string::npos)
		end = cookies.size();

	return trim(cookies.substr(pos, end - pos));
}

SessionManager::SessionManager()
: _ttl(3600)
{
}

void SessionManager::setTtl(size_t ttl)
{
	_ttl = ttl;
}

std::string SessionManager::createSession()
{
	std::string id = generateSessionId();
	Session s;
	s.id = id;
	s.last_access = std::time(NULL);
	_sessions[id] = s;
	return id;
}

Session* SessionManager::getSession(const std::string& id)
{
	std::map<std::string, Session>::iterator it = _sessions.find(id);
	if (it == _sessions.end())
		return NULL;

	if (std::time(NULL) - it->second.last_access > (long)_ttl) {
		_sessions.erase(it);
		return NULL;
	}

	it->second.last_access = std::time(NULL);
	return &it->second;
}

void SessionManager::cleanup() {
	std::map<std::string, Session>::iterator it = _sessions.begin();
	while (it != _sessions.end()) {
		if (std::time(NULL) - it->second.last_access > (long)_ttl) {
			_sessions.erase(it++);
		} else {
			++it;
		}
	}
}

void SessionManager::transferSession(HttpRequest *request, HttpResponse *response)
{
	std::string cookieHeader =  request->getHeader("Cookie");

	std::string sessionId = extractSessionId(cookieHeader);

	Session* session = NULL;
	if (!sessionId.empty()) {
		session = getSession(sessionId);
	}

	if (!session) {
		sessionId = createSession();
		session = getSession(sessionId);
		response->addHeader("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly; SameSite=Lax; Max-Age=3600");
	}
}
