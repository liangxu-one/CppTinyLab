#include "myEvents.h"

int main(int argc, char *argv[])
{
    char *ip = argv[1];
    int port = atoi(argv[2]);

    // 创建监听sockfd
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket error");
        return 0;
    }
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server.sin_addr.s_addr);
    int ret;
    ret = bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    if (ret < 0)
    {
        perror("bind error");
        return 0;
    }

    // 监听
    ret = listen(sockfd, maxConnectNums);
    if (ret < 0)
    {
        perror("listen error");
        return 0;
    }

    // 创建epoll树
    int epfd = epoll_create(maxConnectNums);

    // 创建sockfd节点并上树, 对sockfd监听读事件, 当有事件发生时, 调用其回调函数initAccept
    struct epoll_event ep_event;
    myEvent *p_myEvent = new myEvent(sockfd, initAccept);
    ep_event.events = EPOLLIN;
    ep_event.data.ptr = p_myEvent;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ep_event);
    if (ret < 0)
    {
        perror("epoll_ctl error");
        return 0;
    }

    // 循环等待事件触发
    myEvent *tempMyevent;
    struct epoll_event ep_events[maxConnectNums];
    int nready;
    while (1)
    {
        nready = epoll_wait(epfd, ep_events, maxConnectNums, -1);
        if (nready < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("epoll error");
                break;
            }
        }
        else
        {
            for (int i = 0; i < nready; i++)
            {
                tempMyevent = (myEvent *)ep_events[i].data.ptr;
                tempMyevent->function(epfd, tempMyevent->fd, tempMyevent);
            }
        }
    }

    // 退出时释放资源
    delete p_myEvent;
    close(sockfd);
    close(epfd);
    return 0;
}
