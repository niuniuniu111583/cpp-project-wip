#include "socket.hpp"
#include <arpa/inet.h>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
int writen(int fd, const char *msg, int size)
{
    const char *buf = msg;
    int count = size;
    while (count > 0)
    {
        int len = send(fd, buf, count, 0);
        if (len == -1)
        {
            return -1;
        }
        else if (len == 0)
        {
            continue;
        }
        buf += len;
        count-=len;
    }
    return size;
}

int sendMsg(int cfd,const char* msg,int len){
    if(cfd<0||msg==nullptr||len<0){
        return -1;
    }
    char* data=(char*)malloc(len+4);
    int biglen=htonl(len);
    memcpy(data,&biglen,4);
    memcpy(data+4,msg,len);
    int ret=writen(cfd,data,len+4);
    if(ret==-1){
        close(cfd);
    }
    return ret;
}

//接收指定字节个数字符串
int readn(int fd,char *buf,int size){
     
}