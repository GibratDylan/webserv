#pragma once

#include <string>
#include <map>

typedef std::map<std::string, std::string> HeaderMap;

class Config;

class HttpResponse {
public:
    int statusCode;
    std::string reason;
    HeaderMap headers;
    std::string body;

    HttpResponse();
    HttpResponse(int code, const std::string& reason);
    HttpResponse(const HttpResponse& other);
    HttpResponse& operator=(const HttpResponse& other);

    void addHeader(const std::string& key, const std::string& value);
    std::string build() const;

    static HttpResponse makeResponse(int code, const std::string &type, const std::string& body);
    static HttpResponse makeRedirectResponse(int code, const std::string& url);
    static HttpResponse makeGetResponse(const std::string& path, const Config &config);
    static HttpResponse makePostResponse(const std::string& path, const std::string& body, const Config &config);
    static HttpResponse makeDeleteResponse(const std::string& path, const Config &config);
    static HttpResponse makeFileResponse(const std::string& path, const Config &config);
    static HttpResponse makeErrorResponse(int code, const Config &config);

    static std::map<int, std::string> reasons;
    static void initReasons();
    static std::string getReason(int code);
};
