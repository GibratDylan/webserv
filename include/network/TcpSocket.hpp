#pragma once

#include <sys/types.h>

#include <cstddef>
#include <string>

class TcpSocket {
   public:
	TcpSocket();
	explicit TcpSocket(int fd);
	~TcpSocket();

	void create();
	void bind(const std::string& host, int port);
	void listen(int backlog);

	void setNonBlocking(bool nonBlocking);
	void setReuseAddr(bool reuse);

	TcpSocket* accept();

	ssize_t send(const void* data, size_t len);
	ssize_t receive(void* buffer, size_t len);

	int getFd() const;
	std::string getHost() const;
	int getPort() const;
	bool isListening() const;

	void close();
	int release();

   private:
	TcpSocket(const TcpSocket&);
	TcpSocket& operator=(const TcpSocket&);

	int _fd;
	bool _listening;
	std::string _host;
	int _port;
};
