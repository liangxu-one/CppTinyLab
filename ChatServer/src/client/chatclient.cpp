#include "chatclient.hpp"
#include <thread>

using json = nlohmann::json;
using std::thread;

ChatClient::ChatClient(EventLoop *loop, const InetAddress &serverAddr, const string &nameArg)
    : _client(loop, serverAddr, nameArg), _loop(loop)
{
    // 注册连接回调
    _client.setConnectionCallback(std::bind(&ChatClient::onConnect, this, _1));
    // 注册消息回调
    _client.setMessageCallback(std::bind(&ChatClient::onMessage, this, _1, _2, _3));
}

void ChatClient::connect()
{
    _client.connect();
}

void ChatClient::onConnect(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        // 客户端需要能够同时处理读写消息, 因此这里开启两个线程, 当连接建立后该线程负责控制客户端整体流程, 收到消息后由onMessage处理, 通过信号量实现同步操作
        thread t(&ClientService::getStart, &_clientService, conn, _loop);
        t.detach();
    }
    else
    {
        conn->shutdown();
    }
}

void ChatClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    _clientService.readTaskHandler(conn, buf, time);
}