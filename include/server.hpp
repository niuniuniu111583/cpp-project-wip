#pragma once
#include "channel.hpp"
#include "eventloop.hpp"
#include <vector>

class Server
{
public:
    explicit Server(unsigned short port);
    //启动入口，创建Server对象后，调用此函数，进入持续的事件循环
    void start();
    void sstop();
    ~Server();
private:
    int initListenFd(unsigned short port);
    int listenFd_;
    Channel *listenChannel_;
    EventLoop eventloop_;
    //保存在管理中的客户端 Channel
    std::vector<Channel*>clientChannels_;
    //保存已经断开、等待释放的 Channel
    std::vector<Channel*>retiredChannels_;

    // 当监听套接字有新连接时，调用 accept() 接收它。
    void handleNewConnection();
    //读取客户端数据，判断客户端是否断开
    void handleClientRead(int cfd);
};