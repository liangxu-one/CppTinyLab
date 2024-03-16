#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using std::function;
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);
    // 启动服务
    void start();
    // 连接回调函数
    void onConnect(const TcpConnectionPtr &);
    // 有消息时的读写回调函数
    void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

private:
    // 实现服务器的类对象
    TcpServer _server;
    // 指向事件循环的指针
    EventLoop *_loop;
};