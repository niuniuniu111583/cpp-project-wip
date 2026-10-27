#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include "server.hpp"
#include <unistd.h>
#include <stdexcept>
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

void Server::start(){
    eventloop_.loop();
}

void Server::handleNewConnection()
{
    struct sockaddr_in clientAddr{};
    socklen_t clientLen=sizeof(clientAddr);
    int cfd = accept(listenFd_, (struct sockaddr *)&clientAddr, &clientLen);
    if (cfd == -1)
    {
        perror("accept");
        return;
    }
    cout << "new client connected,cfd=" << cfd << endl;
    //为client创建channel
    Channel* clientChannel=new Channel(cfd);
    //设置client的读事件回调
    clientChannel->setReadCallback([cfd](){
        char buf[1024]={};
        ssize_t len=recv(cfd,buf,sizeof(buf),0);
        if(len>0){
            cout<<"client sent : ";
            cout.write(buf,len);
            cout<<endl; 
        }
        else if(len==0){
            cout<<"client disconnected , cfd = "<<cfd<<endl;
            close(cfd);
        }
        else{
            perror("recv");
        }

    });
    //注册到EventLoop
    eventloop_.addChannel(clientChannel);
    //保存client的Channel
    clientChannels_.push_back(clientChannel);

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
