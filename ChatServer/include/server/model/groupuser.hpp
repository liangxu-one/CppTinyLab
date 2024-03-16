#pragma once

#include "user.hpp"

// 群组用户, 直接继承自User类, 多了role角色信息
class GroupUser : public User
{
public:
    void setRole(string role)
    {
        this->role = role;
    }

    string getRole()
    {
        return this->role;
    }

private:
    string role;
};