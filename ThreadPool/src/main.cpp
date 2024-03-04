#include "threadPool.h"

int main()
{
    int num;
    // 线程池默认构造函数生成2个线程, 最大任务数为5
    ThreadPool p_threadPool;
    while (!p_threadPool.shufdown)
    {
        printf("请输入想要增加的任务数或者输入-1代表销毁线程池:\n");
        scanf("%d", &num);
        if (num == -1)
        {
            p_threadPool.shufdown = true;
        }
        else
        {
            for (int i = 0; i < num; i++)
            {
                addTask(&p_threadPool, myTask(NULL, fun));
            }
        }
    }
    p_threadPool.destoryThreadPool();
    // 等待线程池销毁
    sleep(5);
}