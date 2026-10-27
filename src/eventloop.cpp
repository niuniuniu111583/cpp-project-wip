#include"eventloop.hpp"

EventLoop::EventLoop(){}

void EventLoop::addChannel(Channel* channel){
    int fd=channel->getFd();
    if(epoll_.addFd(fd)){
        channels_[fd]=channel;
    }

}
void EventLoop::loop(){
    while (true)
    {
        vector<int>readyFds=epoll_.waitEvents();
        for(int fd: readyFds){
            auto it =channels_.find(fd);
            if(it!=channels_.end()){
                it->second->handleReadEvent();
            }
        }
    }
    
}