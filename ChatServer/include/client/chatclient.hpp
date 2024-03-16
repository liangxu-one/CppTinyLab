#pragma once

#include "clientservice.hpp"
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <semaphore.h>

using namespace muduo;
using namespace muduo::net;

class ChatClient
{
public:
    // 初始化客户端
    ChatClient(EventLoop *loop, const InetAddress &serverAddr, const string &nameArg);
    // 与服务端建立连接
    void connect();
    // 连接回调函数
    void onConnect(const TcpConnectionPtr &conn);
    // 有消息时回调函数
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

private:
    TcpClient _client;
    EventLoop *_loop;
    ClientService _clientService;
};