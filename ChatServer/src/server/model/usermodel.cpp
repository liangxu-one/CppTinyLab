#include "usermodel.hpp"
#include "db.hpp"
#include <cstring>

bool UserModel::insert(User &user)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "select * from User where id = %d", id);
    MySQL mysql;
    User user;
    if (mysql.connect())
    {
        // 查询登录信息是否存在
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
            }
            mysql_free_result(res);
        }
    }
    return user;
}

bool UserModel::updateState(User &user)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "update User set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    // 组装sql语句
    char sql[1024] = "update User set state = 'offline' where state = 'online'";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}