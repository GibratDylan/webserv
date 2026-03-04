#pragma once

#include <string>
#include <map>

class HttpRequest;
class HttpResponse;

struct Session
{
    std::string id;
    std::map<std::string, std::string> data;
    time_t last_access;
};


class SessionManager
{
private:
    std::map<std::string, Session> _sessions;
    size_t _ttl; 

public:
    SessionManager();

    std::string createSession();
    Session* getSession(const std::string& id);
    void cleanup();
    void setTtl(size_t ttl);
    void transferSession(HttpRequest *request, HttpResponse *response);
};