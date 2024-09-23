#include "netlib.h"
namespace netlib
{
	int init_server(const std::string &address, int port)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addr = {
			AF_INET,
			htons(port)
		};
		inet_pton(AF_INET, address.c_str(), &(addr.sin_addr));
		if (bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1)
		{
			std::println("Bind failed!");
			close(sockfd);
			return -1;
		}
		if (listen(sockfd, 10) == -1)
		{
			std::println("Listen failed!");
			close(sockfd);
			return -1;
		}
		return sockfd;
	}

	int connect_to_server(const std::string &address, int port)
	{
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addr = {
			AF_INET,
			htons(port)
		};
		inet_pton(AF_INET, address.c_str(), &(addr.sin_addr));
		if (connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1)
		{
			std::println("Connect failed!");
			return -1;
		}
		return sock;
	}

	void add_to_list(int fd, int epfd)
	{
		epoll_event event;
		event.data.fd = fd;
		event.events = EPOLLIN;
		epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
	}
	void remove_from_list(int fd, int epfd)
	{
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
	}

	void disconnect_server(int fd, int epfd)
	{
		remove_from_list(fd, epfd);
		std::println("Removed fd {} from epoll", fd);
		close(fd);
	}

}
