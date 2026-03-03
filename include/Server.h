#pragma once

#include <vector>
#include <map>
#include <poll.h>
#include <string>

class Server {
private:
    int _port;
    int _fd;
    std::vector<pollfd> _fds;
    std::map<int, std::string> _buffers;

    void addFd(int fd);
    void setupSocket();
    void acceptClient();
    void handleClient(int fd);

public:
    Server(int port);
    void run();
};

