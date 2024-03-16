#include "offlinemsgmodel.hpp"
#include "db.hpp"
#include <cstring>

void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

void OfflineMsgModel::remove(int userid)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    vector<string> msgVec;
    MySQL mysql;
    if (mysql.connect())
    {
        // 查询登录信息是否存在
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                msgVec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return msgVec;
}