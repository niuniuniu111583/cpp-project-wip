#pragma once
#include<pthread.h>
#include<queue>
using callback=void (*)(void* arg);
class TaskQueue{
private:
    
    callback function;
    void *arg;
};

struct ThreadPool;

//创建线程池
ThreadPool* threadPoolCreate(int min,int max, int queueSize);

//销毁线程池
int threadPoolDestroy(ThreadPool* pool);

//给线程池添加任务
void threadPoolAdd(ThreadPool* pool,void (*func)(void*),void* arg);

//获取线程池工作的线程个数
int threadPoolBusyNum(ThreadPool* pool);

//获取线程池工作的线程个数
int threadPoolAliveNum(ThreadPool* pool);

//
void* worker(void* arg);
void* manager(void* arg);
void* threadExit(ThreadPool* pool);