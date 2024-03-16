#include "groupmodel.hpp"
#include "db.hpp"
#include <cstring>

bool GroupModel::createGroup(Group &group)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的组数据生成的主键id
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "insert into GroupUser values(%d, %d, '%s')",
            groupid, userid, role.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    /*
    1. 根据userid在GroupUser表中查询出该用户所属的群组信息
    2. 在根据群组信息, 查询属于该群组的所有用户的userid, 并且和user表进行多表联合查询, 查询用户的详细信息
    */
    sprintf(sql, "select AllGroup.* \
            from GroupUser, AllGroup \
            where GroupUser.groupid = AllGroup.id and GroupUser.userid = %d",
            userid);

    vector<Group> groupVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    // 查询群组的用户信息
    for (Group &group : groupVec)
    {
        memset(sql, '\0', sizeof(sql));
        sprintf(sql, "select User.id, User.name, User.state, GroupUser.grouprole \
                from User, GroupUser \
                where User.id = GroupUser.userid and GroupUser.groupid = %d",
                group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

vector<int> GroupModel::queryGropuUsers(int userid, int groupid)
{
    char sql[1024];
    memset(sql, '\0', sizeof(sql));
    // 组装sql语句
    sprintf(sql, "select userid from GroupUser where userid != %d and groupid = %d", userid, groupid);

    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}