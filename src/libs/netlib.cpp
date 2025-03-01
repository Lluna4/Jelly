#include "netlib.h"
namespace netlib
{
	int init_server(const std::string &address, int port)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		
		if (inet_pton(AF_INET, address.c_str(), &(addr.sin_addr)) == -1)
		{
			std::println("Inet pton failed! {}", strerror(errno));
			close(sockfd);
			return -1;
		}
		if (bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1)
		{
			std::println("Bind failed! {}", strerror(errno));
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

	void add_to_list(int fd, int kq)
	{
		struct kevent ev;
		EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
		kevent(kq, &ev, 1, NULL, 0, NULL);
	}

	void remove_from_list(int fd, int kq)
	{
		struct kevent ev;
		EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
		kevent(kq, &ev, 1, NULL, 0, NULL);
	}

	void disconnect_server(int fd, int epfd)
	{
		remove_from_list(fd, epfd);
		std::println("Removed fd {} from epoll", fd);
		close(fd);
	}

}
