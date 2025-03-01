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

	#ifdef __APPLE__
	void add_to_list(int fd, int epfd)
	{
		struct kevent ev;
		EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
		kevent(epfd, &ev, 1, NULL, 0, NULL);
	}
	#endif
	#ifdef __linux__
	void add_to_list(int fd, int epfd)
	{
		epoll_event event;
		event.data.fd = fd;
		event.events = EPOLLIN;
		epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
	}
	#endif

	#ifdef __APPLE__
	void remove_from_list(int fd, int epfd)
	{
		struct kevent ev;
		EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
		kevent(epfd, &ev, 1, NULL, 0, NULL);
	}
	#endif
	#ifdef __linux__
	void remove_from_list(int fd, int epfd)
	{
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
	}
	#endif

	void disconnect_server(int fd, int epfd)
	{
		remove_from_list(fd, epfd);
		std::println("Removed fd {} from epoll", fd);
		close(fd);
	}

}
