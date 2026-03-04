#pragma once

#include <vector>
#include <map>
#include <poll.h>
#include "Connection.h"
#include "config/GlobalConfig.hpp"
#include "exceptions.h"
#include "SessionManager.h"


class Server
{
private:
    std::vector<pollfd> _pollFds;
    std::map<int, Connection*> _connections;
    std::map<int, ServerConfig*> _listenSockets;
    SessionManager _sessionManager;

private:
    void setupSockets();
    void addPollFd(int fd, short events);
    void removePollFd(int fd);

    void acceptConnection(int listenFd);
    void removeConnection(int fd);

    void handlePollEvents();
    void checkTimeouts();

public:
    GlobalConfig config;
    Server(std::string& config_file_name);
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();

    void run();
};

