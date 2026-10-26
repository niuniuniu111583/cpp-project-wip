#include "epoll.hpp"
#include <unistd.h>
#include <cstdio>

Epoll::Epoll()
{
    epollFd_ = epoll_create(1);
    if (epollFd_ == -1)
    {
        perror("epoll_create");
    }
}
Epoll::~Epoll()
{
    if (epollFd_ != -1)
    {
        close(epollFd_);
    }
}

bool Epoll::addFd(int fd)
{
    struct epoll_event ev{};
    // 注册事件时保存 fd
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl");
        return false;
    }
    return true;
}
vector<int> Epoll::waitEvents()
{
    // events数组用来接收 epoll_wait() 返回的就绪事件
    struct epoll_event events[1024];
    int num = epoll_wait(epollFd_, events, 1024, -1);

    vector<int> readyFds;
    if (num == -1)
    {
        perror("epoll_wait");
        return readyFds;
    }
    for (int i = 0; i < num; i++)
    {
        // 事件发生后取出 fd
        readyFds.push_back(events[i].data.fd);
    }
    return readyFds;
}