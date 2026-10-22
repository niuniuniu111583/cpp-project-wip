#include <iostream>
#include <pthread.h>
#include "threadpool.hpp"
#include <unistd.h>
#include "server.hpp"
using namespace std;



int main()
{
    int lfd=initListenFd(9999);
    if(lfd==-1){
        return -1;
    }
    getchar();
    close(lfd);
    return 0;
}