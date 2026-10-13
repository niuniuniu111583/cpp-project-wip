#include "threadpool.hpp"
#include <iostream>
#include <pthread.h>
#include <cstring>
#include <unistd.h>
const int NUMBER = 2;
// 任务结构体



typedef struct Task
{
    void (*function)(void *arg); // void (*func)(void*)返回值是void类型，是一个函数指针；；void *func(void*)表示func是一个返回void*的函数
    void *arg;
} Task;

// 线程池结构体
struct ThreadPool
{
    Task *taskQ;
    int queueCapacity;
    int queueSize;
    int queueFront;
    int queueRear;

    pthread_t managerID;
    pthread_t *threadIDs;
    int minNum;
    int maxNum;
    int busyNum; // 忙线程
    int liveNum; // 存活线程
    int exitNum; // 要销毁的线程个数
    pthread_mutex_t mutexpool;
    pthread_mutex_t mutexBusy; // 因为busyNum经常被修改，所以锁
    pthread_cond_t notFull;
    pthread_cond_t notEmpty;

    int shutdown; // 是不是要销毁线程池，销毁为1
};

ThreadPool *threadPoolCreate(int min, int max, int queueSize)
{
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    do
    {
        if (pool == NULL)
        {
            perror("malloc threadpool fail...\n");
            break;
        }
        pool->threadIDs = (pthread_t *)malloc((sizeof(pthread_t)) * max);
        if (pool->threadIDs == NULL)
        {
            perror("malloc threadIDs fail...\n");
            break;
        }
        memset(pool->threadIDs, 0, sizeof(pthread_t) * max); // 初始化
        pool->minNum = min;
        pool->maxNum = max;
        pool->busyNum = 0;
        pool->liveNum = min;
        pool->exitNum = 0;
        if (pthread_mutex_init(&pool->mutexpool, NULL) != 0 || pthread_mutex_init(&pool->mutexBusy, NULL) != 0 || pthread_cond_init(&pool->notEmpty, NULL) != 0 || pthread_cond_init(&pool->notFull, NULL) != 0)
        {
            perror("mutex or cond init fail...\n");
        }
        pool->taskQ = (Task *)malloc(sizeof(Task) * queueSize);
        pool->queueCapacity = queueSize;
        pool->queueFront = 0;
        pool->queueRear = 0;
        pthread_create(&pool->managerID, NULL, manager, pool);
        for (int i = 0; i < min; i++)
        {
            pthread_create(&pool->threadIDs[i], NULL, worker, pool);
        } // create不负责为线程ID分配空间，只负责创建线程并写入线程ID，线程ID必须提前有存储空间，可以是普通变量。可以是malloc动态分配

        pool->shutdown = 0;
        return pool;
    } while (0);
    // 释放资源
    if (pool && pool->threadIDs)
        free(pool->threadIDs); // free掉指向的malloc内存，对应内存被释放，指针悬空
    if (pool && pool->taskQ)
        free(pool->taskQ);
    if (pool)
        free(pool);

    return NULL;
}

void *worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    while (1)
    {
        pthread_mutex_lock(&pool->mutexpool);
        // 当前任务队列是否为空
        while (pool->queueSize == 0 && !pool->shutdown)
        {
            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty, &pool->mutexpool);

            // 判断是否需要销毁线程
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                pool->liveNum--;
                pthread_mutex_unlock(&pool->mutexpool);
                threadExit(pool);
            }
        }
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexpool);
            threadExit(pool);
        }
        Task task;
        task.function = pool->taskQ[pool->queueFront].function;
        task.arg = pool->taskQ[pool->queueFront].arg;
        // 移动头结点(循环)
        pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
        pool->queueSize--;
        pthread_cond_signal(&pool->notFull);
        pthread_mutex_unlock(&pool->mutexpool);

        printf("thread %ld start working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexBusy);

        task.function(task.arg);
        free(task.arg);  // 释放的是arg指向的内存，不是arg本身
        task.arg = NULL; // 让arg不再保存已经失效的地址
        printf("thread %ld end working...\n", pthread_self());

        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexBusy);
    }
    return NULL;
}

void *manager(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    while (!pool->shutdown)
    {
        //
        sleep(3);
        pthread_mutex_lock(&pool->mutexpool);
        int queueSize = pool->queueSize;
        int liveNum = pool->liveNum;
        pthread_mutex_unlock(&pool->mutexpool);

        pthread_mutex_lock(&pool->mutexBusy);
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->mutexBusy);

        // 添加线程(规定任务个数>存活个数&&存活线程数<最大线程数)
        if (queueSize > liveNum && liveNum < pool->maxNum)
        {
            int counter = 0;
            pthread_mutex_lock(&pool->mutexpool);
            for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; i++)
            {
                if (pool->threadIDs[i] == 0)
                {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    counter++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexpool);
        }
        // 销毁线程(忙线程*2<存活线程数&&存活线程>最小线程数)
        if (busyNum * 2 < liveNum && liveNum > pool->maxNum)
        {
            pthread_mutex_lock(&pool->mutexpool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexpool);
            for (int i = 0; i < NUMBER; i++)
            {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return NULL;
}

void *threadExit(ThreadPool *pool)
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < pool->maxNum; i++)
    {
        if (pool->threadIDs[i] == tid)
        {
            pool->threadIDs[i] = 0;
            printf("threadExit() called,%ld exiting...\n", tid);
            break;
        }
    }
    pthread_exit(NULL);
    return NULL;
}

void threadPoolAdd(ThreadPool *pool, void (*func)(void *), void *arg)
{
    pthread_mutex_lock(&pool->mutexpool);
    while (pool->queueSize == pool->queueCapacity && !pool->shutdown)
    {
        // 阻塞生产者线程（任务队列）
        pthread_cond_wait(&pool->notFull, &pool->mutexpool); // wait期间自动释放mutexpool，被唤醒后重新加锁；线程阻塞
    }
    if (pool->shutdown)
    {
        pthread_mutex_unlock(&pool->mutexpool);
        return;
    }
    // 添加任务
    pool->taskQ[pool->queueRear].function = func;
    pool->taskQ[pool->queueRear].arg = arg;
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
    pool->queueSize++;
    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->mutexpool);
}

int threadPoolBusyNum(ThreadPool *pool)
{
    pthread_mutex_lock(&pool->mutexBusy);
    int busyNum = pool->busyNum;
    pthread_mutex_unlock(&pool->mutexBusy);
    return busyNum;
}

int threadPoolAliveNum(ThreadPool *pool)
{
    pthread_mutex_lock(&pool->mutexpool);
    int aliveNum = pool->liveNum;
    pthread_mutex_unlock(&pool->mutexpool);
    return aliveNum;
}

int threadPoolDestroy(ThreadPool *pool)
{
    if (pool == NULL)
    {
        return -1;
    }

    pool->shutdown = 1;
    pthread_join(pool->managerID, NULL);
    // 唤醒阻塞的消费者线程
    for (int i = 0; i < pool->liveNum; i++)
    {
        pthread_cond_broadcast(&pool->notEmpty);
    }
    // 释放堆内存
    if (pool->taskQ)
    {
        free(pool->taskQ);
    }
    if (pool->threadIDs)
    {
        free(pool->threadIDs);
    }
    pthread_mutex_destroy(&pool->mutexBusy);
    pthread_mutex_destroy(&pool->mutexpool);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);
    free(pool);
    pool = NULL;
    return 0;
}