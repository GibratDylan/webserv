#pragma once

#include <string>
#include <vector>
#include <map>
/*
class MyConfig 
{
public:    
    std::string host;
    int port;
    std::string root;
    std::string upload_store;  
    std::vector<std::string> index;
    bool autoindex;
    size_t client_max_body_size;
    size_t large_client_header_buffers; // URI_TOO_LONG 
    size_t client_header_buffer_size;   // HEADER_FIELDS_TOO_LARGE
    std::pair<int, std::string> redirection;                    // 301, 302, 0 if no redirect
    std::map<int, std::string> error_pages;
    std::vector<std::string> methods;

    size_t session_ttl; // in seconds

    MyConfig()
    {
        host = "localhost";
        port = 8080;
        root = "./www/8080";
        upload_store = "./www/uploads";
        index.push_back("index.html");
        autoindex = true;
        client_max_body_size = 10;
        large_client_header_buffers = 8192;
        client_header_buffer_size = 8192;
        redirection = std::make_pair(0, "");  // no redirect
        methods.push_back("GET");
        methods.push_back("POST");
        methods.push_back("DELETE");
        session_ttl = 3600; 

    }
};


class MyServerConfig : public MyConfig
{
public:
    MyServerConfig() 
    { }
    MyConfig resolveConfig(const std::string& location) const
    {
        (void)location;
        MyConfig resolved = *this;
        return resolved;
    }
};

class MyGlobalConfig : public MyConfig
{
public:
    std::vector<MyServerConfig> servers;

    MyGlobalConfig(std::string& config_file_name)
    {
        (void)config_file_name; 
        MyServerConfig s1;
        s1.index.push_back("index.htm");
        servers.push_back(s1);

        MyServerConfig s2;
        s2.port = 9090;
        s2.root = "./www/9090";
        s2.autoindex = false;
        s2.error_pages[404] = "/error/page404.html";
        // s2.redirect_code = 301;
        // s2.redirect_url = "http://localhost:8080/";
        servers.push_back(s2);

        MyServerConfig s3;
        s3.port = 8888;
        s3.root = "./www/8888";
        // s3.autoindex = false;
        s3.error_pages[404] = "/error/page404.html";
        // s2.redirect_code = 301;
        // s2.redirect_url = "http://localhost:8080/";
        s3.methods.clear();
        s3.methods.push_back("GET");

        servers.push_back(s3);


    }

};

*/