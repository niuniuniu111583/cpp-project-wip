#include<stdio.h>
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
using namespace std;
int main(){
    int cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1){
        perror("sokect");
        return -1;
    }

    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(9999);
    inet_pton(AF_INET, "172.20.226.237",&saddr.sin_addr.s_addr);
    int ret=connect(cfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret==-1){
        perror("connect");
        return -1;
    }
    cout << "connect success!" << endl;

    int number=0;
    while(1){
        char buff[1024]={0};
        fgets(buff,sizeof(buff),stdin);

        write(cfd,buff,strlen(buff)+1);
        int len=read(cfd,buff,sizeof(buff));
        if(len==-1){
            perror("read error");
            exit(1);
        }
        printf("read buff=%s\n",buff);
        /*sprintf(buff,"hello，world，%d...\n",number++);
        send(cfd,buff,strlen(buff)+1,0);

        memset(buff,0,sizeof(buff));
        int len=recv(cfd,buff,sizeof(buff),0);
        if(len>0){
            cout<<"server say:"<<buff<<endl;
        }
        else if(len==0){
            cout<<"server断开连接..."<<endl;
            break;
        }
        else{
            perror("recv error");
            break;
        }
        sleep(1);*/
    }

    close(cfd);

    return 0;
}