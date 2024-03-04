#include "webServer.h"

const char * supportHttpMethod = "GET";
const char * supportHttpVersion = "HTTP/1.1";
const char * delimiter = "\r\n";
const int maxConnectNums = 1024;
const int maxBufferNums = 4096;
const int countTimeInterval = 30;
const double activeTimeInterval = 60;
pthread_mutex_t mutex;
std::unordered_map<int, fdInfo> hash;

int setfd(int fd)
{
    int old_flag = fcntl(fd, F_GETFL, 0);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
    return old_flag;
}

void * checkConnect(void * arg)
{
    double diff;
    time_t curTime, lastActiveTime;
    int epfd = *(int *)arg, cfd;

    struct sockaddr_in client;
    char client_ip[16];
    while (1)
    {
        sleep(activeTimeInterval);
        pthread_mutex_lock(&mutex);
        for (std::unordered_map<int, fdInfo>::iterator it = hash.begin(); it != hash.end(); it++)
        {
            // 当连接存在且距离上次请求时间超过间隔时间, 将其关闭
            if (it->second.lastActiveTime != -1)
            {
                cfd = it->first;
                lastActiveTime = it->second.lastActiveTime;
                client = it->second.address;

                curTime = time(NULL);
                diff = difftime(curTime, lastActiveTime);
                if (diff > activeTimeInterval)
                {
                    hash[cfd].lastActiveTime = -1;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
                    close(cfd);
                    memset(client_ip, '\0', sizeof(client_ip));
                    inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip));
                    printf("client ip %s, port %d has been inactive for more than %f seconds since the last activity\n", client_ip, ntohs(client.sin_port), activeTimeInterval);
                }
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void * getConnectCount(void * arg)
{    
    int epfd = *(int *)arg;
    int count;
    while (1)
    {
        count = 0;
        sleep(countTimeInterval);
        pthread_mutex_lock(&mutex);
        for (std::unordered_map<int, fdInfo>::iterator it = hash.begin(); it != hash.end(); it++)
        {
            if (it->second.lastActiveTime != -1)
            {
                count = count + 1;
            }
        }
        pthread_mutex_unlock(&mutex);
        printf("%d clients keep connecting\n", count);
    }
    pthread_exit(NULL);
}

void http_request(int epfd, int cfd)
{
    int n;
    char buf[maxBufferNums];
    memset(buf, '\0', sizeof(buf));
    n = recv(cfd, buf, sizeof(buf), 0);
    if (n <= 0)
    {
        hash[cfd].lastActiveTime = -1;
        epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
        close(cfd);
        perror("read error");
        return;
    }

    // // 循环读取剩余字符
    // char restBuf[maxBufferNums];
    // while (1)
    // {
    //     memset(restBuf, '\0', sizeof(restBuf));
    //     n = recv(cfd, restBuf, sizeof(restBuf), 0);
    //     if (n == -1)
    //     {
    //         break;
    //     }
    //     else if (n <= 0)
    //     {
    //         hash[cfd].lastActiveTime = -1;
    //         epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
    //         close(cfd);
    //         perror("read error");
    //         return;
    //     }
    // }

    // 读取http请求的请求行, 以"\r\n"作为分隔符
    char * line = strtok(buf, delimiter);
    std::vector<char *> lines;
    while (line != NULL) 
    {
        lines.push_back(line);
        line = strtok(NULL, delimiter);
    }

    // 判断请求行的方法与http协议号是否匹配
    char * requestLine = lines[0];
    char reqType[16] = {'\0'}, fileName[256] = {'\0'}, protocal[16] = {'\0'};
    sscanf(requestLine, "%[^ ] %[^ ] %[^\\0]", reqType, fileName, protocal);
    if (strcmp(reqType, supportHttpMethod) != 0 || strcmp(protocal, supportHttpVersion) != 0)
    {
        hash[cfd].lastActiveTime = -1;
        epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
        close(cfd);
        printf("http method error or version error\n");
        return;
    }

    char *pFile = fileName;
    // 判断输入文件路径是否为空或者只有一个"/", 是的话跳转到根目录, 否则跳过"/"获取其文件名称
    if (strlen(fileName) <= 1)
    {
        strcpy(pFile, "./");
    }
    else
    {
        pFile = fileName + 1;
    }
    // 若文件名为中文则有乱码, 进行乱码修复
    strdecode(pFile, pFile);
    int ret;
    struct stat st;

    // 判断文件是否存在, 若不存在发送error.html文件, 否则查看文件是哪种类型, 若为目录则发送目录列表, 否则发送文件内容
    ret = stat(pFile, &st);
    if (ret < 0)
    {
        printf("%s file not exist\n", pFile);
        sendHeader(epfd, cfd, "404", "NOT FOUND", get_mime_type(".html"), 0);
        sendFile(epfd, cfd, "error.html");
    }
    else
    {
        if (S_ISREG(st.st_mode))
        {
            printf("%s file exists\n", pFile);
            sendHeader(epfd, cfd, "200", "OK", get_mime_type(pFile), st.st_size);
            sendFile(epfd, cfd, pFile);
        }
        else
        {
            printf("%s dir exists\n", pFile);
            sendHeader(epfd, cfd, "200", "OK", get_mime_type(".html"), st.st_size);
            sendFile(epfd, cfd, "html/dir_header.html");

            struct dirent **namelist;
            int fileNums;
            char dirBuf[maxBufferNums];
            fileNums = scandir(pFile, &namelist, NULL, alphasort);
            if (fileNums == -1)
            {
                hash[cfd].lastActiveTime = -1;
                epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
                close(cfd);
                perror("scandir error");
                return;
            }

            // 获取目录信息, 并每获取10个文件名发送一次
            fileNums = fileNums - 1;
            while (fileNums >= 0) 
            {
                memset(dirBuf, '\0', sizeof(dirBuf));
                for (int i = 0; i < 10 && fileNums >= 0; i++)
                {
                    if (namelist[fileNums]->d_type == DT_DIR)
                    {
                        sprintf(dirBuf + strlen(dirBuf), "<li><a href=%s/> %s </a></li>", namelist[fileNums]->d_name, namelist[fileNums]->d_name);
                    }
                    else
                    {
                        sprintf(dirBuf + strlen(dirBuf), "<li><a href=%s> %s </a></li>", namelist[fileNums]->d_name, namelist[fileNums]->d_name);
                    }
                    free(namelist[fileNums]);
                    fileNums = fileNums - 1;
                }
                send(cfd, dirBuf, strlen(dirBuf), 0);
            }
            free(namelist);
            sendFile(epfd, cfd, "html/dir_tail.html");
        }
    }
}

int sendHeader(int epfd, int cfd, const char * code, const char * msg, const char * fileType, int length)
{
    char buf[maxBufferNums] = {'\0'};
    sprintf(buf, "%s %s %s\r\n", supportHttpVersion, code, msg);
    sprintf(buf + strlen(buf), "Content-Type:%s\r\n", fileType);
    if (length > 0)
    {
        sprintf(buf + strlen(buf), "Content-Length:%d\r\n", length);
    }
    strcat(buf, "\r\n");
    send(cfd, buf, strlen(buf), 0);
    return 0;
}

int sendFile(int epfd, int cfd, const char * fileNmae)
{
    int fd = open(fileNmae, O_RDONLY);
    if (fd < 0)
    {
        perror("open error");
        return -1;
    }
    int n;
    char buf[maxBufferNums];
    while (1)
    {
        memset(buf, '\0', strlen(buf));
        n = read(fd, buf, sizeof(buf));
        if (n <= 0)
        {
            break;
        }
        else
        {
            send(cfd, buf, n, 0);
        }
    }
    close(fd);
    return 0;
}

const char * get_mime_type(const char * name)
{
    //自右向左查找‘.’字符;如不存在返回NULL
    const char * dot = strrchr(name, '.');
    /*
     *charset=iso-8859-1	西欧的编码, 说明网站采用的编码是英文；
     *charset=gb2312		说明网站采用的编码是简体中文；
     *charset=utf-8			代表世界通用的语言编码；
     *						可以用到中文、韩文、日文等世界上所有语言编码上
     *charset=euc-kr		说明网站采用的编码是韩文；
     *charset=big5			说明网站采用的编码是繁体中文；
     *
     *以下是依据传递进来的文件名, 使用后缀判断是何种文件类型
     *将对应的文件类型按照http定义的关键字发送回去
     */
    if (dot == (char*)0)
        return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp( dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

unsigned short hextod(unsigned short hexNums)
{
    unsigned short num;
    if (hexNums >= '0' && hexNums <= '9')
    {
        num = hexNums - '0';
    }
    else if (hexNums >= 'a' && hexNums <= 'f')
    {
        num = hexNums - 'a' + 10;
    }
    else if (hexNums >= 'A' && hexNums <= 'F')
    {
        num = hexNums - 'A' + 10;
    }
    return num;
}

void strdecode(char * to, char * from)
{
    int len = strlen(from);
    for (; *from != '\0';)
    {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            *to = hextod(from[1]) * 16 + hextod(from[2]);
            to = to + 1;
            from = from + 3;
        }
        else
        {
            *to = *from;
            to = to + 1;
            from = from + 1;
        }
    }
    *to = '\0';
    for (; to != from; to++)
    {
        *to = '\0';
    }
}