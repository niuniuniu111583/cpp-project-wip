#pragma once
#include "channel.hpp"
#include "epoll.hpp"
#include <unordered_map>
using namespace std;

class EventLoop
{
public:
    EventLoop();
    // 把一个 Channel 加入管理，同时把它的 fd 注册到 epoll
    void addChannel(Channel *channel);
    //// 把一个 Channel 从 EventLoop 中移除
    bool removeChannel(int fd);

    // 负责反复等待事件，并将事件分发给对应的 Channel
    void loop();
    // 请求停止事件循环
    void stop();

private:
    // 创建EventLoop对象时，创建epoll实例
    Epoll epoll_;
    // fd 与 Channel 的对应关系
    unordered_map<int, Channel *> channels_;
    // 控制事件循环是否继续运行
    bool running_ = true;
};
