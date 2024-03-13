#include "webServer.h"

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGPIPE, &act, NULL);

    hash.clear();
    pthread_mutex_init(&mutex, NULL);

    // 改变web服务器的工作目录
    char path[256] = {'\0'};
    getcwd(path, sizeof(path));
    strcat(path, "/../webpath");
    chdir(path);

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket error");
        return 0;
    }
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv.sin_addr.s_addr);
    int ret;
    ret = bind(sockfd, (struct sockaddr *)&serv, sizeof(serv));
    if (ret < 0)
    {
        perror("bind error");
        close(sockfd);
        return 0;
    }

    ret = listen(sockfd, 128);
    if (ret < 0)
    {
        perror("listen error");
        return 0;
    }

    int epfd = epoll_create(maxConnectNums);
    if (epfd < 0)
    {
        perror("epoll_create error");
        close(sockfd);
        return 0;
    }

    pthread_t pt1, pt2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&pt1, &attr, checkConnect, &epfd);
    if (ret < 0)
    {
        printf("%s\n", strerror(ret));
        return 0;
    }
    ret = pthread_create(&pt2, &attr, getConnectCount, &epfd);
    if (ret < 0)
    {
        printf("%s\n", strerror(ret));
        return 0;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret < 0)
    {
        perror("epoll_ctl error");
        close(sockfd);
        return 0;
    }

    struct sockaddr_in client;
    socklen_t client_len;
    char client_ip[16];

    int cfd, nready;
    struct epoll_event events[maxConnectNums];
    while (1)
    {
        nready = epoll_wait(epfd, events, maxConnectNums, -1);
        if (nready < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("epoll_wait error");
                break;
            }
        }
        else
        {
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < nready; i++)
            {
                if (events[i].data.fd == sockfd)
                {
                    bzero(&client, sizeof(client));
                    cfd = accept(sockfd, (struct sockaddr *)&client, &client_len);
                    ev.events = EPOLLIN;
                    ev.data.fd = cfd;
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                    if (ret < 0)
                    {
                        perror("epoll_ctl error");
                        close(cfd);
                        return 0;
                    }
                    hash[cfd].lastActiveTime = time(NULL);
                    hash[cfd].address = client;
                    // setfd(cfd);
                    memset(client_ip, '\0', sizeof(client_ip));
                    printf("cfd: %d, ip: %s, port: %d\n", cfd, inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip)), ntohs(client.sin_port));
                }
                else
                {
                    cfd = events[i].data.fd;
                    hash[cfd].lastActiveTime = time(NULL);
                    http_request(epfd, cfd);
                }
            }
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutex);
    close(sockfd);
    close(epfd);
    return 0;
}
