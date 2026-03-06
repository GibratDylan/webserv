#pragma once

#include <string>
#include <ctime>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "config/ServerConfig.hpp"

class SessionManager;
class Session;

class Connection
{
public:
    enum State
    {
        READING,
        PROCESSING,
        WRITING,
        DONE
    };

private:
    int _fd;
    State _state;

    std::string _readBuffer;
    std::string _writeBuffer;

    HttpRequest  _request;
    HttpResponse _response;

    time_t _lastActivity;

private:
    void readFromSocket();
    void processRequest();
    void reset();

public:
    Connection(int fd, ServerConfig *cfg, SessionManager* sessionManager);
    ~Connection();

    State getState() const;
    void onRead();
    void onWrite();
    bool isDone() const;
    bool isTimeout(time_t now) const;
    void handleSession();

public:
    ServerConfig *config;
    SessionManager* _sessionManager;
    Session* _session;
};
