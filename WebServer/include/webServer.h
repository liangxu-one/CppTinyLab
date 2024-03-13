#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct fdInfo
{
    time_t lastActiveTime;      // 上次活跃时间
    struct sockaddr_in address; // 客户端连接地址
};
extern const char *supportHttpMethod;   // 支持的http方法
extern const char *supportHttpVersion;  // 支持的http版本
extern const char *delimiter;           // http行与行之间的分隔符
extern const int maxConnectNums;        // 最大连接数(初始化事件数组)
extern const int maxBufferNums;         // 读写数据时使用的数组大小
extern const int countTimeInterval;     // 获取当前连接数的间隔时间
extern const double activeTimeInterval; // 检查当前连接是否活跃的间隔时间
extern pthread_mutex_t mutex;
extern std::unordered_map<int, fdInfo> hash; // 记录客户端连接的相关信息

// 设置文件描述符为非阻塞
int setfd(int fd);
// 打印当前的连接数目
void *checkConnect(void *arg);
// 检查连接是否活跃
void *getConnectCount(void *arg);
// 请求分析
void http_request(int epfd, int cfd);
// 发送头文件
int sendHeader(int epfd, int cfd, const char *code, const char *msg, const char *fileType, int length);
// 发送响应内容
int sendFile(int epfd, int cfd, const char *fileNmae);
// 获取相应时需要的文件类型
const char *get_mime_type(const char *name);
// 十六进制转十进制
unsigned short hextod(unsigned short hexNums);
// 解决中文乱码
void strdecode(char *to, char *from);