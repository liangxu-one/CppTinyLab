#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>

// 处理服务器异常结束后, 重置业务信息
void resetHandler(int sigNo)
{
    ChatService::getInstance()->reset();
    exit(0);
}

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cout << "没有指定ip地址与端口号" << std::endl;
        return 0;
    }
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = resetHandler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    EventLoop loop;
    InetAddress addr(argv[1], atoi(argv[2]));
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();
    return 0;
}