#pragma once
#include "channel.hpp"
#include "eventloop.hpp"

class Server
{
public:
    explicit Server(unsigned short port);
    //启动入口，创建Server对象后，调用此函数，进入持续的事件循环
    void start();

private:
    int initListenFd(unsigned short port);
    int listenFd_;
    Channel *listenChannel_;
    EventLoop eventloop_;

    // 当监听套接字有新连接时，调用 accept() 接收它。
    void handleNewConnection();
};