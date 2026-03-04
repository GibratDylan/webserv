#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "Connection.h"
#include "utils.h"
#include "FileHandler.h"

Connection::Connection(int fd, ServerConfig *cfg, SessionManager* sessionManager)
: _fd(fd), _state(READING), _request(this), config(cfg), _sessionManager(sessionManager)
{
    _session = NULL;
    _lastActivity = std::time(NULL);
}

Connection::~Connection()
{
    close(_fd);
}

Connection::State Connection::getState() const
{
    return _state;
}

bool Connection::isDone() const
{
    return _state == DONE;
}

bool Connection::isTimeout(time_t now) const
{
    return (now - _lastActivity > 30); 
}

void Connection::reset()
{
    _readBuffer.clear();
    _writeBuffer.clear();
    _request = HttpRequest(this);
    _response = HttpResponse();
    _state = READING;
}

void Connection::readFromSocket()
{
    char buffer[4096];
    ssize_t bytes;

    while ((bytes = recv(_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        _readBuffer.append(buffer, bytes);
        _lastActivity = std::time(NULL);
    }

    if (bytes == 0)
    {
        _state = DONE;
        return;
    }

    if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        _state = DONE;
    }
}

void Connection::onWrite()
{
    if (_state != WRITING)
        return;

    if (_writeBuffer.empty())
        return;

    int sent = send(_fd, _writeBuffer.c_str(), _writeBuffer.size(), 0);

    if (sent > 0)
    {
        _writeBuffer.erase(0, sent);
        _lastActivity = std::time(NULL);
    }
    else if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        _state = DONE;
        return;
    }

    if (_writeBuffer.empty())
    {
        if (_request.headers.count("Connection") && _request.headers.at("Connection") == "keep-alive")
            reset();
        else
            _state = DONE;
    }
}

void Connection::onRead()
{
    if (_state != READING)
        return;

    readFromSocket();

    if (_state == DONE)
        return;

    processRequest();
}

void Connection::processRequest()
{
    ParseStatus status = _request.parse(_readBuffer);
    if (status == PARSE_INCOMPLETE)
        return;
    if (status != PARSE_OK)
    {
        _response = HttpResponse::makeErrorResponse(status, *config);
        _writeBuffer = _response.build();
        _state = WRITING;
        return;
    }

    _state = PROCESSING;
    Config resolvedConfig = config->resolveConfig(_request.path);
    
    if (resolvedConfig.redirection.first != 0)
         _response = HttpResponse::makeRedirectResponse(resolvedConfig.redirection.first, resolvedConfig.redirection.second);
    else if (_request.method == "GET")
        _response = HttpResponse::makeGetResponse(_request.path, resolvedConfig);
    else if (_request.method == "POST")
        _response = HttpResponse::makePostResponse(_request.path, _request.body, resolvedConfig);
    else if (_request.method == "DELETE")
        _response = HttpResponse::makeDeleteResponse(_request.path, resolvedConfig);
    else
        _response = HttpResponse::makeErrorResponse(405, *config);

    _writeBuffer = _response.build();
    _state = WRITING;
}
