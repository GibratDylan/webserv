#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>

#include "../../include/network/IOMultiplexer.hpp"
#include "../../include/network/TcpSocket.hpp"

static int getBoundPort(int fd) {
	sockaddr_in addr;
	socklen_t len = sizeof(addr);
	std::memset(&addr, 0, sizeof(addr));
	if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
		return -1;
	}
	return ntohs(addr.sin_port);
}

static void testIOMultiplexerPipeReadable() {
	int fds[2];
	int ret = ::pipe(fds);
	assert(ret == 0);

	IOMultiplexer mux;
	mux.addFd(fds[0], POLLIN);

	const char byte = 'x';
	ssize_t w = ::write(fds[1], &byte, 1);
	assert(w == 1);

	ret = mux.poll(50);
	assert(ret >= 1);
	assert(mux.size() == 1);
	assert(mux.getFd(0) == fds[0]);
	assert((mux.getRevents(0) & POLLIN) != 0);

	::close(fds[0]);
	::close(fds[1]);
}

static void testTcpSocketAcceptSendReceive() {
	TcpSocket listenSock;
	listenSock.create();
	listenSock.setReuseAddr(true);
	listenSock.bind("127.0.0.1", 0);
	listenSock.listen(16);

	int port = getBoundPort(listenSock.getFd());
	assert(port > 0);

	int clientFd = ::socket(AF_INET, SOCK_STREAM, 0);
	assert(clientFd >= 0);

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast<unsigned short>(port));
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int c =
		::connect(clientFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	assert(c == 0);

	TcpSocket* serverSock = listenSock.accept();
	assert(serverSock != NULL);

	const char msg[] = "hello";
	ssize_t sent = ::send(clientFd, msg, sizeof(msg), 0);
	assert(sent == static_cast<ssize_t>(sizeof(msg)));

	char buf[32];
	std::memset(buf, 0, sizeof(buf));
	ssize_t r = -1;
	for (int i = 0; i < 200; ++i) {
		r = serverSock->receive(buf, sizeof(buf));
		if (r > 0) {
			break;
		}
		if (r < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			break;
		}
		::usleep(1000);
	}
	assert(r == static_cast<ssize_t>(sizeof(msg)));
	assert(std::memcmp(buf, msg, sizeof(msg)) == 0);

	const char reply[] = "world";
	ssize_t s2 = -1;
	for (int i = 0; i < 200; ++i) {
		s2 = serverSock->send(reply, sizeof(reply));
		if (s2 > 0) {
			break;
		}
		if (s2 < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			break;
		}
		::usleep(1000);
	}
	assert(s2 == static_cast<ssize_t>(sizeof(reply)));

	char buf2[32];
	std::memset(buf2, 0, sizeof(buf2));
	ssize_t rr = ::recv(clientFd, buf2, sizeof(buf2), 0);
	assert(rr == static_cast<ssize_t>(sizeof(reply)));
	assert(std::memcmp(buf2, reply, sizeof(reply)) == 0);

	delete serverSock;
	::close(clientFd);
}

int main() {
	try {
		testIOMultiplexerPipeReadable();
		testTcpSocketAcceptSendReceive();
	} catch (...) {
		return 1;
	}
	std::cout << "[OK] network unit tests" << '\n';
	return 0;
}
