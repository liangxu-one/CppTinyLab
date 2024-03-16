#pragma once

#include "groupuser.hpp"
#include <vector>

using std::vector;

// Group的映射类
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    void setId(int id)
    {
        this->id = id;
    }

    int getId()
    {
        return this->id;
    }

    void setName(string name)
    {
        this->name = name;
    }

    string getName()
    {
        return this->name;
    }

    void setDesc(string desc)
    {
        this->desc = desc;
    }

    string getDesc()
    {
        return this->desc;
    }

    vector<GroupUser> &getUsers()
    {
        return this->users;
    }

private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};