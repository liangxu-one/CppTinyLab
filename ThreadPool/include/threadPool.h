#include <iostream>
#include <queue>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

class myTask
{
public:
    void * arg;
    void * (*fun)(void * arg);
    myTask(void * arg = NULL, void * (*fun)(void * arg) = NULL)
    {
        this->arg = arg;
        this->fun = fun;
    }
};

class ThreadPool
{
public:
    bool shufdown;

    int maxTaskNums;
    int maxThreadNums;
    std::queue<myTask> taskQueue;

    sem_t full_sem;
    sem_t empty_sem;
    pthread_mutex_t mutex;
    pthread_t * p_th;

    ThreadPool(int maxThreadNums = 2, int maxTaskNums = 5);
    void destoryThreadPool();
};

void * threadProcess(void * arg);
void addTask(ThreadPool * p_threadPool, myTask task);
void * fun(void * arg);