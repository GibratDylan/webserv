#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include "HttpRequest.h"
#include "Connection.h"
#include "utils.h"

HttpRequest::HttpRequest(Connection *connection)
: _complete(false), _connection(connection)
{
}

HttpRequest::HttpRequest(const HttpRequest& other)
: _complete(other._complete), _connection(other._connection), method(other.method), path(other.path),
  version(other.version), headers(other.headers), body(other.body)
{
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
    if (this != &other)
    {
        _complete = other._complete;
        method = other.method;
        path = other.path;
        version = other.version;
        headers = other.headers;
        body = other.body;
    }
    return *this;
}

bool HttpRequest::isComplete() const 
{
    return _complete;
}

std::string HttpRequest::getHeader(const std::string& name) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(name);
    if (it != headers.end())
        return it->second;
    return "";
}

ParseStatus HttpRequest::parse(const std::string& buffer)
{
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return PARSE_INCOMPLETE; 

    if (headerEnd > (size_t)_connection->config->client_header_buffer_size)
        return HEADER_FIELDS_TOO_LARGE;

    std::string strHeader = buffer.substr(0, headerEnd);
    std::istringstream str(strHeader);
    std::string line;

    if (!std::getline(str, line))
        return BAD_REQUEST;

    if (line[line.size()-1] == '\r')
        line.erase(line.size()-1);

    std::istringstream firstLine(line);
    firstLine >> method >> path >> version;
    if (method.empty() || path.empty() || version.empty())
        return BAD_REQUEST;

    if (path.size() > (size_t)_connection->config->large_client_header_buffers)
        return URI_TOO_LONG;
std::cout << "Request: " << method << " " << path << ". Allowed methods:" ;
    if (std::find(_connection->config->methods.begin(), _connection->config->methods.end(), method) == _connection->config->methods.end())
        return METHOD_NOT_ALLOWED;

for (std::vector<std::string>::const_iterator it = _connection->config->methods.begin(); it != _connection->config->methods.end(); ++it)
    std::cout << " " << *it ;
std::cout << std::endl;

    // if (method != "GET" && method != "POST" && method != "DELETE")
    //     return METHOD_NOT_ALLOWED;
    if (version != "HTTP/1.1" && version != "HTTP/1.0")
        return HTTP_VERSION_NOT_SUPPORTED;

    while (std::getline(str, line))
    {
        if (line == "\r")
            break;

        if (line[line.size()-1] == '\r')
            line.erase(line.size()-1);

        size_t delim = line.find(":");
        if (delim != std::string::npos)
        {
            std::string key = line.substr(0, delim);
            std::string value = line.substr(delim + 2);
            headers[key] = value;
        }
    }

    // if (method == "POST" && !headers.count("Content-Length"))
    //     return LENGTH_REQUIRED;    

    if (headers.count("Content-Length"))
    {
        std::string lenStr = headers["Content-Length"];
        if (!isNumber(lenStr))
            return BAD_REQUEST;

        size_t len = std::atoi(lenStr.c_str());
        if (len > _connection->config->client_max_body_size)
            return PAYLOAD_TOO_LARGE;

        std::string strBody = buffer.substr(headerEnd + 4);

        if (strBody.size() < len)
            return PARSE_INCOMPLETE; 
        body = strBody.substr(0, len);
    }

    _complete = true;
    return PARSE_OK;
}