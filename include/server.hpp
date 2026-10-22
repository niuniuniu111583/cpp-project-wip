#pragma once
#include "channel.hpp"

class Server
{
public:
    explicit Server(unsigned short port);

private:
    int listenFd_;
    Channel *listenChannel_;
};