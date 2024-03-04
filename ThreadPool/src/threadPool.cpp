#include "threadPool.h"

ThreadPool::ThreadPool(int maxThreadNums, int maxTaskNums)
{
    this->shufdown = false;
    this->maxTaskNums = maxTaskNums;
    this->maxThreadNums = maxThreadNums;

    sem_init(&this->empty_sem, 0, maxTaskNums);
    sem_init(&this->full_sem, 0, 0);
    pthread_mutex_init(&this->mutex, NULL);

    this->p_th = new pthread_t[maxThreadNums];
    for (int i = 0; i < this->maxThreadNums; i++)
    {
        pthread_create(&this->p_th[i], NULL, threadProcess, this);
        pthread_detach(this->p_th[i]);
    }
}

void ThreadPool::destoryThreadPool()
{
    this->shufdown = true;
    for (int i = 0; i < this->maxThreadNums; i++)
    {
        sem_post(&this->full_sem);
    }
    delete [] this->p_th;
    sem_destroy(&this->empty_sem);
    sem_destroy(&this->full_sem);
    pthread_mutex_destroy(&this->mutex);
}

void * threadProcess(void * arg)
{
    myTask task;
    ThreadPool * p_threadPool = (ThreadPool *)arg;
    while (!p_threadPool->shufdown)
    {
        sem_wait(&p_threadPool->full_sem);
        pthread_mutex_lock(&p_threadPool->mutex);
        if (p_threadPool->shufdown)
        {
            pthread_mutex_unlock(&p_threadPool->mutex);
            break;
        }
        task = p_threadPool->taskQueue.front();
        p_threadPool->taskQueue.pop();
        pthread_mutex_unlock(&p_threadPool->mutex);
        sem_post(&p_threadPool->empty_sem);
        task.fun(task.arg);
    }
    printf("%ld thread is exit\n", pthread_self());
    pthread_exit(NULL);
}

void addTask(ThreadPool * p_threadPool, myTask task)
{
    if (!p_threadPool->shufdown)
    {
        sem_wait(&p_threadPool->empty_sem);
        pthread_mutex_lock(&p_threadPool->mutex);
        if (p_threadPool->shufdown)
        {
            pthread_mutex_unlock(&p_threadPool->mutex);
            return;
        }
        p_threadPool->taskQueue.push(task);
        pthread_mutex_unlock(&p_threadPool->mutex);
        sem_post(&p_threadPool->full_sem);
    }
}

void * fun(void * arg)
{
    printf("This is a fun, it is execute by %ld thread\n", pthread_self());
    sleep(1);
    return NULL;
}