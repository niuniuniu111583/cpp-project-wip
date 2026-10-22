#pragma once
#include <iostream>
#include <functional>

class Channel
{
public:
    using EventCallback = std::function<void()>;

    // 绑定这个Channel所管理的fd
    explicit Channel(int fd);
    int getFd()const;
    // 设置读事件回调
    void setReadCallback(EventCallback cb);
    // 处理读事件
    void handleReadEvent();

private:
    int fd_;
    EventCallback readCallback_;
};