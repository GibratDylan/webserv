#include <sstream>
#include <cstdlib>
#include <ctime>
#include "SessionManager.h"
#include "HttpRequest.h"
#include "HttpResponse.h"


std::string generateSessionId()
{
    std::stringstream ss;
    ss << std::time(NULL) << rand();
    return ss.str();
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

    if (std::time(NULL) - it->second.last_access > (long)_ttl)
    {
        _sessions.erase(it);
        return NULL;
    }

    it->second.last_access = std::time(NULL);
    return &it->second;
}


std::string extractSessionId(const std::string& cookies)
{
    const std::string sessionKey = "session_id=";
    size_t pos = cookies.find(sessionKey);
    if (pos == std::string::npos)
        return "";

    pos += sessionKey.size();
    size_t end = cookies.find(';', pos);

    if (end == std::string::npos)
        return cookies.substr(pos);

    return cookies.substr(pos, end - pos);
}

void SessionManager::transferSession(HttpRequest *request, HttpResponse *response)
{
    std::string cookies = request->getHeader("Cookie");

    std::string sessionId = extractSessionId(cookies);

    Session* session = getSession(sessionId);

    if (!session)
    {
        std::string newId = createSession();
        session = getSession(newId);
        response->addHeader("Set-Cookie", "session_id=" + newId + "; Path=/; HttpOnly");
    }
}
