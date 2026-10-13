#include <iostream>
#include <sys/epoll.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
using namespace std;

typedef struct socketinfo
{
    int fd;
    int epfd;
} SocketInfo;

void *acceptConn(void *arg)
{
    cout << "acceptconn tid:" << pthread_self() << endl;
    SocketInfo *info = (SocketInfo *)arg;
    int cfd = accept(info->fd, NULL, NULL);
    // 设置非阻塞属性
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);
    struct epoll_event ev; // 可以不写，用上面的ev，因为ev 传给 epoll_ctl() 后，内核会把需要的注册信息保存下来。因此，后面修改 ev，不会把之前注册的事件信息覆盖掉。
    // 复用 ev 没问题，因为它只是提交注册信息的临时结构体；真正维持各个 fd 的注册状态的是内核中的 epoll 实例。
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = cfd;
    epoll_ctl(info->epfd, EPOLL_CTL_ADD, cfd, &ev);
    free(info);
    return NULL;
}

void *communication(void *arg)
{
    cout << "communication tid:" << pthread_self() << endl;
    SocketInfo *info = (SocketInfo *)arg;
    char buf[5];
    string temp;
    bzero(temp, sizeof(temp));
    while (1)
    {
        int len = recv(info->fd, buf, sizeof(buf), 0);

        if (len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                cout << "数据已经接收完毕..." << endl;
                send(info->fd, temp.c_str(), temp.size(), 0);
                break;
            }
            else
            {
                perror("recv error");
                exit(1);
            }
        }
        else if (len == 0)
        {
            cout << "client 断开连接..." << endl;
            epoll_ctl(info->epfd, EPOLL_CTL_DEL, info->fd, NULL);
            close(info->fd);
            break;
        }
        cout << "read buf:" ;
        cout.write(buf,len);
        cout<<endl;
        for (int i = 0; i < len; i++)
        {
            buf[i] = toupper(buf[i]);
        }
        temp.append(buf,len);
        write(STDOUT_FILENO, buf, len);
        cout << endl;
    }
    free(info);
    return NULL;
}

int main(int argc, const char *argv[])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket error");
        return 1; // exit(1)直接退出main，跳过局部对象析构，可能造成局部对象资源泄露
    }
    struct sockaddr_in serv_addr; // ipv4地址结构体
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret == -1)
    {
        perror("bind error");
        return 1;
    }
    ret = listen(lfd, 64);
    if (ret == -1)
    {
        perror("listen error");
        return 1;
    }

    int epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll_create error");
        return 1;
    }
    struct epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(evs[0]);

    while (1)
    {
        int num = epoll_wait(epfd, evs, size, -1);
        cout << "epoll_wait返回的num值 = " << num << endl;
        pthread_t tid;
        for (int i = 0; i < num; i++)
        {
            SocketInfo *info = (SocketInfo *)malloc(sizeof(SocketInfo));
            int fd = evs[i].data.fd; // epoll_ctl() 注册时，内核会保存你设置的关联数据；epoll_wait() 返回时，再把这份关联数据放进 evs 对应的元素里。
            info->epfd = epfd;
            info->fd = fd;
            if (fd == lfd)
            {
                pthread_create(&tid, NULL, acceptConn, info);
                pthread_detach(tid);
            }
            else
            {
                pthread_create(&tid, NULL, communication, info);
                pthread_detach(tid);
            }
        }
    }

    close(lfd);
    return 0;
}