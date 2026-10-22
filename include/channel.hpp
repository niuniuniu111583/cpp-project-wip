#pragma once
#include <iostream>
#include <functional>

class Channel
{
public:
    using EventCallback = std::function<void()>;
    void setReadCallback(EventCallback cb);
    void handleReadEvent();
private:
    EventCallback readCallback_;
};