#pragma once
#include <sys/epoll.h>
#include <stdlib.h>
#include <iostream>

# define MAX_EVENTS 1024

struct packet_def
{
	packet_def(std::vector<const std::type_info *> type, std::vector<std::any> value, User user, char packet_id)
		:types(type), values(value), user_(user), packet_id_(packet_id)
	{}
	
	std::vector<const std::type_info *> types;
	std::vector<std::any> values;
	User user_;
	char packet_id_;
};

int create_instance()
{
    return epoll_create1(0);
}

void add_to_list(int fd, int epfd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

void remove_from_list(int fd, int epfd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
}

