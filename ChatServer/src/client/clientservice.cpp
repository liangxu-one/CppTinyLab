#include "clientservice.hpp"
#include <iostream>
#include <chrono>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

ClientService::ClientService()
{
    isMainMenuRunning = false;
    sem_init(&rwsem, 0, 0);
    g_isLoginSuccess = false;

    commandMap.insert({"help", "显示所有支持的命令, 格式help"});
    commandMap.insert({"chat", "一对一聊天, 格式chat:friendid:message"});
    commandMap.insert({"addfriend", "添加好友, 格式addfriend:friendid"});
    commandMap.insert({"creategroup", "创建群组, 格式creategroup:groupname:groupdesc"});
    commandMap.insert({"addgroup", "加入群组, 格式addgroup:groupid"});
    commandMap.insert({"groupchat", "群聊, 格式groupchat:groupid:message"});
    commandMap.insert({"logout", "注销, 格式logout"});

    commandHandlerMap.insert({"help", std::bind(&ClientService::help, this, _1, _2)});
    commandHandlerMap.insert({"chat", std::bind(&ClientService::chat, this, _1, _2)});
    commandHandlerMap.insert({"addfriend", std::bind(&ClientService::addfriend, this, _1, _2)});
    commandHandlerMap.insert({"creategroup", std::bind(&ClientService::creategroup, this, _1, _2)});
    commandHandlerMap.insert({"addgroup", std::bind(&ClientService::addgroup, this, _1, _2)});
    commandHandlerMap.insert({"groupchat", std::bind(&ClientService::groupchat, this, _1, _2)});
    commandHandlerMap.insert({"logout", std::bind(&ClientService::logout, this, _1, _2)});
}

void ClientService::readTaskHandler(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    string buffer = buf->retrieveAllAsString();
    // 接收ChatServer转发的数据, 反序列化生成json数据对象
    json js = json::parse(buffer);
    int msgtype = js["msgid"].get<int>();
    if (ONE_CHAT_MSG == msgtype)
    {
        cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
             << " said: " << js["msg"].get<string>() << endl;
    }

    if (GROUP_CHAT_MSG == msgtype)
    {
        cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
             << " said: " << js["msg"].get<string>() << endl;
    }

    if (LOGIN_MSG_ACK == msgtype)
    {
        doLoginResponse(js); // 处理登录响应的业务逻辑
        sem_post(&rwsem);    // 通知主线程线程, 登录结果处理完成
    }

    if (REG_MSG_ACK == msgtype)
    {
        doRegResponse(js);
        sem_post(&rwsem); // 通知主线程, 注册结果处理完成
    }
}

string ClientService::getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

void ClientService::getStart(const TcpConnectionPtr &conn, EventLoop *loop)
{
    while (1)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice)
        {
        case 1: // login业务
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); // 读掉缓冲区残留的回车
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            conn->send(js.dump());

            g_isLoginSuccess = false;

            sem_wait(&rwsem); // 等待信号量, 由onMessage线程处理完登录的响应消息后, 通知这里

            if (g_isLoginSuccess)
            {
                // 进入聊天主菜单页面
                isMainMenuRunning = true;
                mainMenu(conn);
            }
        }
        break;
        case 2: // register业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50);
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            conn->send(js.dump());

            sem_wait(&rwsem); // 等待信号量, 子线程处理完注册消息会通知
        }
        break;
        case 3: // quit业务
            loop->quit();
            sem_destroy(&rwsem);
            exit(0);
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }
}

void ClientService::doRegResponse(json &responsejs)
{
    if (0 != responsejs["errno"].get<int>()) // 注册失败
    {
        cerr << "name is already exist, register error!" << endl;
    }
    else // 注册成功
    {
        cout << "name register success, userid is " << responsejs["id"]
             << ", do not forget it!" << endl;
    }
}

void ClientService::doLoginResponse(json &responsejs)
{
    if (0 != responsejs["errno"].get<int>()) // 登录失败
    {
        cerr << responsejs["errmsg"] << endl;
        g_isLoginSuccess = false;
    }
    else // 登录成功
    {
        // 记录当前用户的id和name
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // 记录当前用户的好友列表信息
        if (responsejs.contains("friends"))
        {
            // 初始化
            g_currentUserFriendList.clear();

            vector<string> vec = responsejs["friends"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        // 记录当前用户的群组列表信息
        if (responsejs.contains("groups"))
        {
            // 初始化
            g_currentUserGroupList.clear();

            vector<string> vec1 = responsejs["groups"];
            for (string &groupstr : vec1)
            {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                vector<string> vec2 = grpjs["users"];
                for (string &userstr : vec2)
                {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }

                g_currentUserGroupList.push_back(group);
            }
        }

        // 显示登录用户的基本信息
        showCurrentUserData();

        // 显示当前用户的离线消息  个人聊天信息或者群组消息
        if (responsejs.contains("offlinemsg"))
        {
            vector<string> vec = responsejs["offlinemsg"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                // time + [id] + name + " said: " + xxx
                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
                else
                {
                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }
        g_isLoginSuccess = true;
    }
}

void ClientService::showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

void ClientService::mainMenu(const TcpConnectionPtr &conn)
{
    help(conn, "");

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调, mainMenu对修改封闭, 添加新功能不需要修改该函数
        it->second(conn, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}

void ClientService::help(const TcpConnectionPtr &conn, string str)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void ClientService::addfriend(const TcpConnectionPtr &conn, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    conn->send(js.dump());
}

void ClientService::chat(const TcpConnectionPtr &conn, string str)
{
    int idx = str.find(":"); // friendid:message
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    conn->send(js.dump());
}

void ClientService::creategroup(const TcpConnectionPtr &conn, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["desc"] = groupdesc;
    conn->send(js.dump());
}

void ClientService::addgroup(const TcpConnectionPtr &conn, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    conn->send(js.dump());
}

void ClientService::groupchat(const TcpConnectionPtr &conn, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    conn->send(js.dump());
}

void ClientService::logout(const TcpConnectionPtr &conn, string str)
{
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();

    conn->send(js.dump());
    isMainMenuRunning = false;
    g_isLoginSuccess = false;
}