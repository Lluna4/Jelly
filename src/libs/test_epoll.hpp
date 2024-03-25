#pragma once
#include <sys/epoll.h>
#include <stdlib.h>
#include <iostream>

# define MAX_EVENTS 1024

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