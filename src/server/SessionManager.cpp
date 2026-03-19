#include "../../include/server/SessionManager.hpp"

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../../include/http/HttpResponse.hpp"
#include "../../include/server/HttpRequest.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/Logger.hpp"

std::string toHex(unsigned long long value) {
	std::ostringstream ss;
	ss << std::hex << value;
	return ss.str();
}

std::string generateSessionId() {
	static unsigned long long counter = 0;
	static bool seeded = false;

	if (!seeded) {
		std::srand(static_cast<unsigned int>(std::time(NULL)));
		seeded = true;
	}

	++counter;
	unsigned long long r =
		(static_cast<unsigned long long>(std::rand()) << 32) ^
		static_cast<unsigned long long>(std::rand());
	unsigned long long t =
		(static_cast<unsigned long long>(std::time(NULL)) << 20) ^ counter ^
		static_cast<unsigned long long>(std::rand());
	return toHex(r) + toHex(t);
}

std::string extractSessionId(const std::string& cookies) {
	std::string sessionKey = "session_id=";
	size_t pos = cookies.find(sessionKey);
	if (pos == std::string::npos) return "";

	pos += sessionKey.size();
	size_t end = cookies.find(';', pos);
	if (end == std::string::npos) end = cookies.size();

	return trim(cookies.substr(pos, end - pos));
}

SessionManager::SessionManager() : _ttl(3600) {}

void SessionManager::setTtl(size_t ttl) {
	_ttl = ttl;
	Logger::info(std::string(" Session TTL set to ") + toString(_ttl) + "s");
}

std::string SessionManager::createSession() {
	std::string id = generateSessionId();
	while (_sessions.find(id) != _sessions.end()) id = generateSessionId();

	Session s;
	s.id = id;
	s.last_access = std::time(NULL);
	_sessions[id] = s;
	Logger::debug(std::string(" Session created id=") + id);
	return id;
}

Session* SessionManager::getSession(const std::string& id) {
	std::map<std::string, Session>::iterator it = _sessions.find(id);
	if (it == _sessions.end()) return NULL;

	if (std::time(NULL) - it->second.last_access > (long)_ttl) {
		Logger::debug(std::string(" Session expired id=") + id);
		_sessions.erase(it);
		return NULL;
	}

	it->second.last_access = std::time(NULL);
	return &it->second;
}

void SessionManager::cleanup() {
	size_t removed = 0;
	std::map<std::string, Session>::iterator it = _sessions.begin();
	while (it != _sessions.end()) {
		if (std::time(NULL) - it->second.last_access > (long)_ttl) {
			removed++;
			_sessions.erase(it++);
		} else {
			++it;
		}
	}
	if (removed > 0) {
		Logger::debug(std::string(" Session cleanup removed=") +
					  toString(removed));
	}
}

void SessionManager::transferSession(HttpRequest* request,
									 HttpResponse* response) {
	std::string cookieHeader = request->getHeader("Cookie");

	std::string sessionId = extractSessionId(cookieHeader);

	Session* session = NULL;
	if (!sessionId.empty()) {
		session = getSession(sessionId);
	}

	if (!session) {
		sessionId = createSession();
		session = getSession(sessionId);
		response->addHeader(
			"Set-Cookie",
			"session_id=" + sessionId +
				"; Path=/; HttpOnly; SameSite=Lax; Max-Age=" + toString(_ttl));
		Logger::debug(std::string(" Session cookie set id=") + sessionId);
	} else {
		Logger::debug(std::string(" Session reused id=") + sessionId);
	}
}
