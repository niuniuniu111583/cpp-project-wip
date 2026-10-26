#include "channel.hpp"
using namespace std;

Channel::Channel(int fd)
{
    fd_ = fd;
}
int Channel::getFd() const
{
    return fd_;
}
void Channel::setReadCallback(EventCallback cb)
{
    readCallback_ = cb;
}
void Channel::handleReadEvent()
{
    if (readCallback_)
    {
        readCallback_();
    }
} // Channel 不决定读事件具体要做什么，而是由外部把处理函数交给它。