All about webserv : https://www.alimnaqvi.com/blog/webserv

Explaination of sockets : https://youtu.be/D26sUZ6DHNQ?si=D45KSx338Ajkvc2V

socketpair :
	- pipes are unidirectional, so you need two pipes to have bidirectional communication, whereas a socketpair is bidirectional.
    - pipes are always stream-oriented, whereas socketpairs can be datagram-oriented.
    - socketpairs are normal AF_UNIX sockets, which means that ancillary messages like SCM_RIGHTS and SCM_CREDENTIALS can be passed over them.
	- In the kernel, pipes are implemented in the filesystem code and socketpairs in the networking code.

