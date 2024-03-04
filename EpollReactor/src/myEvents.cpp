#include "myEvents.h"

extern const int maxConnectNums = 1024;
extern const int maxBufferNums = 1024;

myEvent::myEvent(int fd, void (*function)(int epfd, int fd, void * arg))
{
    this->n = 0;
    this->buf = new char[maxBufferNums];
    memset(this->buf, '\0', maxBufferNums);
    this->fd = fd;
    this->function = function;
}

myEvent::~myEvent()
{
    delete [] this->buf;
}

void readData(int epfd, int fd, void * arg)
{
    // 读取数据
    myEvent * p_myEvent = (myEvent *)arg;
    memset(p_myEvent->buf, '\0', maxBufferNums);
    p_myEvent->n = recv(fd, p_myEvent->buf, maxBufferNums, 0);
    if (p_myEvent->n <= 0)
    {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        perror("read error");
        delete p_myEvent;
        return;
    }

    // 需要进行的处理(此处可自定义为其他方法)
    for (int i = 0; i < p_myEvent->n; i++)
    {
        p_myEvent->buf[i] = toupper(p_myEvent->buf[i]);
    }

    // 将该fd上的监听事件修改为写事件, 并将回调函数设置为wirteData
    struct epoll_event new_ep_event;
    p_myEvent->function = writrData;
    new_ep_event.events = EPOLLOUT;
    new_ep_event.data.ptr = p_myEvent;
    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &new_ep_event);
    if (ret < 0)
    {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        perror("epoll_ctl error");
        delete p_myEvent;
        return;
    }
}

void writrData(int epfd, int fd, void * arg)
{
    // 发送数据
    myEvent * p_myEvent = (myEvent *)arg;
    int n = send(fd, p_myEvent->buf, p_myEvent->n, 0);
    if (n <= 0)
    {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        perror("write error");
        delete p_myEvent;
        return;
    }

    // 将该fd上的监听事件修改为写事件, 并将回调函数设置为readData
    struct epoll_event new_ep_event;
    p_myEvent->function = readData;
    new_ep_event.events = EPOLLIN;
    new_ep_event.data.ptr = p_myEvent;
    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &new_ep_event);
    if (ret < 0)
    {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        perror("epoll_ctl error");
        delete p_myEvent;
        return;
    }
}

void initAccept(int epfd, int fd, void * arg)
{
    // 阻塞等待连接
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    bzero(&client, sizeof(client));
    int cfd = accept(fd, (struct sockaddr *)&client, &client_len);
    if (cfd < 0)
    {
        perror("accept error");
        return;
    }

    // 接受新连接后查看对方ip与port
    char client_ip[16];
    memset(client_ip, '\0', sizeof(client_ip));
    inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip));
    printf("client ip: %s, client port: %d\n", client_ip, ntohs(client.sin_port));

    // 将该fd上树, 其监听事件为读事件
    struct epoll_event ep_event;
    myEvent * p_myEvent = new myEvent(cfd, readData);
    ep_event.events = EPOLLIN;
    ep_event.data.ptr = p_myEvent;

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ep_event);
    if (ret < 0)
    {
        close(fd);
        perror("epoll_ctl error");
        delete p_myEvent;
        return;
    }
}