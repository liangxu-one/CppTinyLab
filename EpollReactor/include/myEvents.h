#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern const int maxConnectNums;
extern const int maxBufferNums;

// 自定义事件类, epoll_event中的data.ptr指向该类生成的对象
class myEvent
{
public:
    int n;
    int fd;
    char *buf;

    void (*function)(int epfd, int fd, void *arg);
    myEvent(int fd = 0, void (*function)(int epfd, int fd, void *arg) = nullptr);
    ~myEvent();
};

// 回调函数
void initAccept(int epfd, int fd, void *arg);
void readData(int epfd, int fd, void *arg);
void writrData(int epfd, int fd, void *arg);