#ifndef NETLIB_H
#define NETLIB_H
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <print>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>

namespace netlib
{
	int init_server(const std::string &address, int port);
	int connect_to_server(const std::string &address, int port);
	void add_to_list(int fd, int epfd);
	void remove_from_list(int fd, int epfd);
	void disconnect_server(int fd, int epfd);
}

#endif //NETLIB_H
