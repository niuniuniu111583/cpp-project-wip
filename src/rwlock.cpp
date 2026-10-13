#include<stdio.h>
#include<pthread.h>
#include<iostream>
#include<unistd.h>
#include "rwlock.hpp"
#define MAX 50
using namespace std;
int number;
pthread_rwlock_t rwlock;

void* read_num(void* arg){
    for(int i=0;i<MAX;++i){
        pthread_rwlock_rdlock(&rwlock);
        cout<<"Thread read,id="<<pthread_self()<<" number="<<number<<endl; 
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

void* write_num(void*){
    for(int i=0;i<MAX;++i){
        pthread_rwlock_wrlock(&rwlock);
        int cur=number;
        cur++;
        number=cur;
        cout<<"Thread write,id="<<pthread_self()<<" number="<<number<<endl; 
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

int main(int argc,const char *argv[]){
    pthread_t p1[5],p2[3];
    pthread_rwlock_init(&rwlock,NULL);

    for(int i=0;i<5;i++){
        pthread_create(&p1[i],NULL,read_num,NULL);
    }
    for(int i=0;i<3;i++){
        pthread_create(&p2[i],NULL,write_num,NULL);
    }

    for(int i=0;i<5;i++){
        pthread_join(p1[i],NULL);
    }
    for(int i=0;i<3;i++){
        pthread_join(p2[i],NULL);
    }

    pthread_rwlock_destroy(&rwlock);

    return 0;
}