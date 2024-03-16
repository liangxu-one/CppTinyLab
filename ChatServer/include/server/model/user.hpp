#pragma once

#include <string>

using std::string;

// User的映射表
class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
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

    void setPassword(string password)
    {
        this->password = password;
    }

    string getPassword()
    {
        return this->password;
    }

    void setState(string state)
    {
        this->state = state;
    }

    string getState()
    {
        return this->state;
    }

private:
    int id;
    string name;
    string password;
    string state;
};