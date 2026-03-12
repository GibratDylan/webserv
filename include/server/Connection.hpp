#pragma once

#include <ctime>
#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class SessionManager;
struct Session;
class CgiHandler;
class ServerConfig;

class Connection {
   public:
	enum State { READING, PROCESSING, WRITING, DONE, PROCESSING_CGI, INIT_CGI };

   private:
	int _fd;
	State _state;

	std::string _readBuffer;
	std::string _writeBuffer;

	HttpRequest _request;
	HttpResponse _response;

	time_t _lastActivity;

   private:
	void readFromSocket();
	void processRequest();
	void reset();

   public:
	Connection(int fd, const ServerConfig& cfg, SessionManager& sessionManager);
	~Connection();

	State getState() const;
	void setState(State state);
	void onRead();
	void onWrite();
	bool isDone() const;
	bool isTimeout(time_t now) const;
	void handleSession();
	void finalizeCgi();

   public:
	const ServerConfig& config;
	SessionManager& sessionManager;
	CgiHandler* cgi;
};
