#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
using namespace std;

sem_t semp;
sem_t semc;
pthread_mutex_t mutex;

struct Node
{
    int number;
    struct Node* next;
};
Node* head=NULL;

void* producer(void* arg){
    while(1){
        sem_wait(&semp);
        pthread_mutex_lock(&mutex);
        Node* newNode=(struct Node*)malloc(sizeof(struct Node));
        newNode->number=rand()%100;
        newNode->next=head;
        head=newNode;
        cout<<"生产者,id:"<<pthread_self()<<"  number:"<<newNode->number<<endl;
        pthread_mutex_unlock(&mutex);
        sem_post(&semc);
        sleep(1);
    }
    return NULL;
}
void* consumer(void* arg){
    while(1){
        sem_wait(&semc);
        pthread_mutex_lock(&mutex);
        struct Node* node=head;
        cout<<"消费者,id:"<<pthread_self()<<"  number:"<<node->number<<endl;
        head=head->next;
        free(node);
        pthread_mutex_unlock(&mutex);
        sem_post(&semp);
        sleep(1);
    }
    return NULL;
}

int main(){
    sem_init(&semp,0,5);
    sem_init(&semc,0,0);
    pthread_mutex_init(&mutex,NULL);

    pthread_t t1[5],t2[5];
    for(int i=0;i<5;i++){
        pthread_create(&t1[i],NULL,producer,NULL);
    }
    for(int i=0;i<5;i++){
        pthread_create(&t2[i],NULL,consumer,NULL);//把新线程ID写入此地址，所以需要传值
    }
    for(int i=0;i<5;i++){
        pthread_join(t1[i],NULL);//join是根据线程ID找到线程并等待，不需要写入，所以只传值
    }
    for(int i=0;i<5;i++){
        pthread_join(t2[i],NULL);
    }

    sem_destroy(&semp);
    sem_destroy(&semc);
    pthread_mutex_destroy(&mutex);
    return 0;
}