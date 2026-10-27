#pragma once
#include <sys/epoll.h>
#include <vector>
using namespace std;

class Epoll
{
public:
    Epoll();
    ~Epoll();
    // 把一个fd注册到epoll中
    bool addFd(int fd);
    //把fd从epoll中移除
    bool rmFd(int fd);
    // 等待事件发生，并返回就绪的fd列表
    vector<int> waitEvents();

private:
    // 保存 epoll_create() 创建的 epoll 实例对应的文件描述符
    int epollFd_;
};
