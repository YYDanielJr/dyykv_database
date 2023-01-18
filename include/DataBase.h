#pragma once
// 这个文件用来处理数据库的读写。
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "Logger.h"

extern Logger logger;

class database // 统一管理数据库内存数据的读写
{
private:
    struct node // 结点
    {
        char val;
        std::vector<node *> next;
        std::string value_node;
    };
    // 在思考了很久之后，我决定用一种类似于字典树的方式进行内存中的存储。
    // 之所以说是类似于字典树的方式，是因为完全意义上的字典树的孩子都是直接开一个26位大小的数组，然后直接用数字表示孩子的字母的。
    // 这样的搜索方式是最快的，但是对于键值数据库来说，键不只是26个字母，如果我直接开一个ASCII码大小的数组内存量会上天的。
    // 所以改用了vector存储，这样的话搜索的速度会变慢，但是不至于用链表或者哈希表那么慢。
    // 我会在内存中用这种树的方式进行存储，在硬盘中只存储一对一对的字符串对，在每次开启数据库的时候先把硬盘上的数据加载到树里。
    // 这是我暂时能想到的最大化节省内存操作时间和硬盘操作时间的方法。
    node *root;
    std::string temp_value;

public:
    database()
    {
        root = new node;
        root->val = 0;
    }
    ~database()
    {
        node *p = root;
        deleteTree(p);
        root = nullptr;
    }
    void deleteTree(node *p)
    {
        for (int i = 0; i < p->next.size(); i++)
        {
            deleteTree(p->next.at(i));
        }
        delete p;
    }
    bool put_db(std::string &, std::string &);
    bool delete_db(std::string &);
    std::string get_db(std::string &);
    std::vector<std::string> traverse();                              // 遍历整棵树，是给rw保存文件的接口，返回一个存有string的vector的引用
    void search_node(node *, std::string, std::vector<std::string> &); // 用于遍历的递归函数，遍历到的键值对保存到vector中
};

class rw // 用来统一管理数据库文件的读写
{
private:
    std::ofstream db_writer;
    std::ofstream deletelist_writer;
    std::ifstream db_reader;
    std::ifstream deletelist_reader;
public:
    rw()
    {
        db_reader.open("dyykvdb.db", std::ios::in);
        if(!db_reader.is_open())
        {
            logger.info("Local database is not exist. A new database file will be created.");
        }
        db_writer.open("dyykvdb.db", std::ios::app);
        if(!db_reader.is_open())
        {
            db_reader.open("dyykvdb.db", std::ios::in);
        }

        deletelist_reader.open("deletelist", std::ios::in);
        if(deletelist_reader.is_open())
        {
            logger.info("Server seemed to be shut down unexpectedly last time. We will try to restore your data soon.");
            deletelist_reader.close();
        }
        deletelist_writer.open("deletelist", std::ios::app);
        deletelist_reader.open("deletelist", std::ios::in);
    }
    ~rw()
    {
        db_reader.close();
        db_writer.close();
        deletelist_reader.close();
        deletelist_writer.close();
    }
    bool if_deletelist_null()
    {
        char c;
        deletelist_reader >> c;
        if(deletelist_reader.eof())
        {
            db_reader.seekg(std::ios::beg);
            return true;
        }
        else
        {
            db_reader.seekg(std::ios::beg);
            return false;
        } 
    }
    bool rw_put(std::string &, std::string &);
    bool rw_delete(std::string &);
    // 这里的删除会比put和get复杂得多。
    // 因为按照我的构想，在文件内的数据就只是键-值这样一对一对地简单排放，没有数据结构。
    // 我的想法是，如果有删除命令，先在内存中把那个key的映射删除掉，然后建立一个deletelist文件。
    // 如果是正常的关闭数据库的操作，程序会比对本地数据和deletelist，从本地数据库中删除掉deletelist中存在的键值对，然后删掉deletelist文件。
    // 如果运行过程中被意外重启，那么deletelist文件就不会被删除，这样可以在开启数据库的时候进行上述的删除操作。
    // 因为这种遍历方式是最耗时的，所以我会把它从这里独立出来，缩短两个accept之间的时间。

    // 1.10更新：
    // 妈的，这么搞其实蛮烦的。
    // 现在我的想法是，deletelist还会有，存储方式也依旧是这样的存储方式，但是在保存的时候，如果一切正常，会直接通过遍历树的方式保存字段。
    // 如果出现问题，开启程序的时候deletelist应该也会存在，这样的话先加载原先的数据，然后根据deletelist把内存中的已加载的数据进行处理。
    std::vector<std::string> load();
    std::vector<std::string> load_deletelist();
    void save(std::vector<std::string> &);
};

class database_manager
{
private:
    database *db;
    rw *database_file_manager;

public:
    database_manager()
    {
        db = nullptr;
        database_file_manager = nullptr;
    }
    void start_database_recorder();
    void shutdown_database_recorder();
    bool put_f(std::string &, std::string &);
    bool delete_f(std::string &); // 直接用delete会和c++的关键字重合
    std::string get_f(std::string &);
};