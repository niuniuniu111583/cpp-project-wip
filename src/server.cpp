#include <iostream>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<cstdio>
#include<unistd.h>
using namespace std;
int initListenFd(unsigned short port)
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
    struct sockaddr_in addr{};//初始化为0
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret==-1){
        perror("bind");
        close(lfd);
        return -1;
    }
    ret=listen(lfd,128);
    if(ret==-1){
        perror("listen");
        close(lfd);
        return -1;
    }
    return lfd;
}