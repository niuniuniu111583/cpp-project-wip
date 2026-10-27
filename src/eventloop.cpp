#include "eventloop.hpp"

EventLoop::EventLoop() {}

void EventLoop::addChannel(Channel *channel)
{
    int fd = channel->getFd();
    if (epoll_.addFd(fd))
    {
        channels_[fd] = channel;
    }
}
bool EventLoop::removeChannel(int fd)
{
    if (!epoll_.rmFd(fd))
    {
        return false;
    }
    channels_.erase(fd);
    return true;
}

void EventLoop::loop()
{
    while (running_)
    {
        vector<int> readyFds = epoll_.waitEvents();
        for (int fd : readyFds)
        {
            auto it = channels_.find(fd);
            if (it != channels_.end())
            {
                it->second->handleReadEvent();
            }
        }
    }
}

void EventLoop::stop(){
    running_=false;
}