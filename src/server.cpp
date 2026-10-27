#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include "server.hpp"
#include <unistd.h>
#include <stdexcept>
#include <algorithm>
using namespace std;

Server::Server(unsigned short port)
{
    listenFd_ = initListenFd(port);
    if (listenFd_ == -1)
    {
        throw runtime_error("initListenFd error");
    }

    // 使用 listenFd_ 创建 Channel 对象，并将对象的地址保存到 listenChannel_。
    listenChannel_ = new Channel(listenFd_);
    // 把监听 Channel 注册给 EventLoop
    eventloop_.addChannel(listenChannel_);

    listenChannel_->setReadCallback([this]()
                                    {
        cout<<"new connection event!"<<endl;
        handleNewConnection(); });

    cout << "Server initialized,lfd=" << listenFd_ << endl;
}

void Server::start()
{
    eventloop_.loop();
}

Server::~Server()
{
    // 释放仍然活动的客户端
    for (auto channel : clientChannels_)
    {
        close(channel->getFd());
        delete channel;
    }
    // 释放已经断开的客户端
    for (auto channel : retiredChannels_)
    {
        delete channel;
    }
    // 释放监听 Channel 和监听 fd
    if (listenChannel_ != NULL)
    {
        close(listenFd_);
        delete listenChannel_;
    }
}

void Server::sstop(){
    eventloop_.stop();
}

void Server::handleNewConnection()
{
    struct sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int cfd = accept(listenFd_, (struct sockaddr *)&clientAddr, &clientLen);
    if (cfd == -1)
    {
        perror("accept");
        return;
    }
    cout << "new client connected,cfd=" << cfd << endl;
    // 为client创建channel
    Channel *clientChannel = new Channel(cfd);
    // 设置client的读事件回调
    clientChannel->setReadCallback([this, cfd]()
                                   { handleClientRead(cfd); });
    // 注册到EventLoop
    eventloop_.addChannel(clientChannel);
    // 保存client的Channel
    clientChannels_.push_back(clientChannel);
}

void Server::handleClientRead(int cfd)
{
    char buf[1024] = {};
    ssize_t len = recv(cfd, buf, sizeof(buf), 0);
    if (len > 0)
    {
        string message(buf,len);
        if(message=="quit"||message=="quit\n"||message=="quit\r\n"){
            cout<<"stop command received"<<endl;
            sstop();
            return ;
        }

        cout << "client sent : ";
        cout.write(buf, len);
        cout << endl;
    }
    else if (len == 0)
    {
        cout << "client disconnected , cfd = " << cfd << endl;
        eventloop_.removeChannel(cfd);
        close(cfd);
        // 暂时不 delete Channel 对象。因为当前回调就是由这个 Channel 调用的。先保证它不会再被 epoll 触发，避免在回调执行过程中直接销毁对象。后续再完善对象的释放。
        //// 找到这个客户端对应的 Channel
        auto it = find_if(clientChannels_.begin(), clientChannels_.end(), [cfd](Channel *channel)
                          { return channel->getFd() == cfd; });
        // 从活动列表移到待释放列表
        if (it != clientChannels_.end())
        {
            retiredChannels_.push_back(*it);
            clientChannels_.erase(it);
        }
    }
    else
    {
        perror("recv");
    }
}

int Server::initListenFd(unsigned short port)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        return -1;
    }
    // 设置端口复用
    int opt = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1)
    {
        perror("setsocketopt");
        close(lfd);
        return -1;
    }
    struct sockaddr_in addr{}; // 初始化为0
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind");
        close(lfd);
        return -1;
    }
    ret = listen(lfd, 128);
    if (ret == -1)
    {
        perror("listen");
        close(lfd);
        return -1;
    }
    return lfd;
}
