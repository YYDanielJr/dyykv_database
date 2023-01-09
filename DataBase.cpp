#include "include/DataBase.h"
rw manager; //一个全局对象，所以在使用的时候记得加锁

// --------------------内存管理的成员函数------------------------

bool database::put_db(std::string& key, std::string& value)
{
    node* p = root;
    for(int i = 0; i < key.length(); i++)
    {
        bool if_exist = false;
        for(int j = 0; j < p->next.size(); j++)
        {
            if(p->next.at(i)->val == key[i])
            {
                p = p->next.at(i);
                if_exist = true;
                break;
            }
        }
        if(!if_exist)
        {
            node* temp = new node;
            temp->val = key[i];
            p->next.push_back(temp);
            p = temp;
        }
    }
    p->value_node = value;
    return true;
}

bool database::delete_db(std::string& key)
{
    node* p = root;
    for(int i = 0; i < key.length(); i++)
    {
        if(p->next.at(i)->val == key[i])
        {
            p = p->next.at(i);
        }
        else
        {
            return false;
        }
    }
    p->value_node.clear();
    return true;
}

std::string database::get_db(std::string& key)
{
    node* p = root;
    std::string returner;
    for(int i = 0; i < key.length(); i++)
    {
        if(p->next.at(i)->val == key[i])
        {
            p = p->next.at(i);
        }
        else
        {
            return returner;
        }
    }
    return p->value_node;
}

// --------------------数据库本地文件管理相关函数--------------------



// --------------------接口函数--------------------------

bool put_f(std::string& key, std::string& value)
{

}

bool delete_f(std::string& key)
{

}

std::string get_f(std::string& key)
{
    
}