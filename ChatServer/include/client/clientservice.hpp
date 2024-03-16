#pragma once

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"
#include "json.hpp"
#include <string>
#include <unordered_map>
#include <functional>
#include <semaphore.h>
#include <atomic>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
using std::atomic_bool;
using std::function;
using std::string;
using std::unordered_map;
// 表示指令的事件回调方法类型
using commandHandler = function<void(const TcpConnectionPtr &, string)>;

class ClientService
{
public:
    ClientService();
    // 接收线程
    void readTaskHandler(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    // 获取系统时间（聊天信息需要添加时间信息）
    string getCurrentTime();
    // 系统初始界面
    void getStart(const TcpConnectionPtr &conn, EventLoop *loop);
    // 处理注册的响应逻辑
    void doRegResponse(json &responsejs);
    // 处理登录的响应逻辑
    void doLoginResponse(json &responsejs);
    // 显示当前登录成功用户的基本信息
    void showCurrentUserData();
    // 主聊天页面程序
    void mainMenu(const TcpConnectionPtr &conn);

    // "help" command handler
    void help(const TcpConnectionPtr &conn, string str);
    // "chat" command handler
    void chat(const TcpConnectionPtr &conn, string str);
    // "addfriend" command handler
    void addfriend(const TcpConnectionPtr &conn, string str);
    // "creategroup" command handler
    void creategroup(const TcpConnectionPtr &conn, string str);
    // "addgroup" command handler
    void addgroup(const TcpConnectionPtr &conn, string str);
    // "groupchat" command handler
    void groupchat(const TcpConnectionPtr &conn, string str);
    // "logout" command handler
    void logout(const TcpConnectionPtr &conn, string str);

private:
    // 记录当前系统登录的用户信息
    User g_currentUser;
    // 记录当前登录用户的好友列表信息
    vector<User> g_currentUserFriendList;
    // 记录当前登录用户的群组列表信息
    vector<Group> g_currentUserGroupList;
    // 控制主菜单页面程序
    bool isMainMenuRunning;
    // 用于读写线程之间的通信
    sem_t rwsem;
    // 记录登录状态
    atomic_bool g_isLoginSuccess;
    // 系统支持的客户端命令列表
    unordered_map<string, string> commandMap;
    // 注册系统支持的客户端命令处理
    unordered_map<string, commandHandler> commandHandlerMap;
};