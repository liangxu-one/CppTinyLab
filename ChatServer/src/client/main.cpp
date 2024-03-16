#include "chatclient.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "没有指定ip地址与端口号" << std::endl;
        return 0;
    }

    EventLoop loop;
    InetAddress addr(argv[1], atoi(argv[2]));
    ChatClient client(&loop, addr, "ChatClient");

    client.connect();
    loop.loop();

    return 0;
}