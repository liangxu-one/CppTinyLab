#include "db.hpp"
#include <muduo/base/Logging.h>

const string mysqlHost = "127.0.0.1";
const string mysqlUser = "root";
const string mysqlPassword = "123456";
const string mysqlDbName = "chat";

MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, mysqlHost.c_str(), mysqlUser.c_str(), mysqlPassword.c_str(), mysqlDbName.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        mysql_set_character_set(_conn, "utf8");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }
    return p != nullptr;
}

bool MySQL::update(string sql)
{
    int ret = mysql_query(_conn, sql.c_str());
    if (ret != 0)
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败";
        return false;
    }
    return true;
}

MYSQL_RES *MySQL::query(string sql)
{
    int ret = mysql_query(_conn, sql.c_str());
    if (ret != 0)
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败";
    }
    return mysql_use_result(_conn);
}

MYSQL *MySQL::getConnection()
{
    return _conn;
}