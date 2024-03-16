#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnect, this, _1));
    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程数量
    _server.setThreadNum(2);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnect(const TcpConnectionPtr &conn)
{
    // 如果服务器断开连接
    if (!conn->connected())
    {
        ChatService::getInstance()->clinetCloseException(conn);
        conn->shutdown();
    }
};

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 数据的反序列化
    json js = json::parse(buf);
    // 完全解耦网络模块的代码和业务模块的代码
    // 通过js["msgid"]获取=>业务handler=>调用该handler执行业务逻辑
    MsgHandler msgHandler = ChatService::getInstance()->getMsgHandle(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器, 执行相应的业务处理
    msgHandler(conn, js, time);
};