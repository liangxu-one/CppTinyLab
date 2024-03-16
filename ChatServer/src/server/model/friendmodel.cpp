#include "friendmodel.hpp"
#include "db.hpp"
#include <cstring>

void FriendModel::insert(int userid, int friendid)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "insert into Friend values(%d, %d)", userid, friendid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
        // 双方互相成为好友
        memset(sql, '\0', sizeof(sql));
        sprintf(sql, "insert into Friend values(%d, %d)", friendid, userid);
        mysql.update(sql);
    }
};

vector<User> FriendModel::query(int userid)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "select User.id, User.name, User.state \
            from User, Friend \
            where User.id = Friend.friendid and Friend.userid = %d",
            userid);

    vector<User> msgVec;
    MySQL mysql;
    if (mysql.connect())
    {
        // 查询好友信息
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 读取userid用户的所有好友信息
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                msgVec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return msgVec;
}