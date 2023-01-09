#pragma once
// 这个文件用来处理数据库的读写。
#include <string>
#include <mutex>
#include <vector>
#include <cstdio>
#include "Logger.h"
Logger db_logger;
std::recursive_mutex db_mtx;    //用的是递归锁，因为在两个logger类的内部已经加过锁了，可能会出现重复上锁的问题。
//同样的，两个logger类的内部也改成递归锁了。
std::mutex rw_mtx;  //我会再建立一个读写文件的类，这个锁是给这个类用的

class database  //统一管理数据库内存数据的读写
{
private:
    struct node //结点
    {
        char val;
        std::vector<node*> next;
        std::string value_node;
    };
    // 在思考了很久之后，我决定用一种类似于字典树的方式进行内存中的存储。
    // 之所以说是类似于字典树的方式，是因为完全意义上的字典树的孩子都是直接开一个26位大小的数组，然后直接用数字表示孩子的字母的。
    // 这样的搜索方式是最快的，但是对于键值数据库来说，键不只是26个字母，如果我直接开一个ASCII码大小的数组内存量会上天的。
    // 所以改用了vector存储，这样的话搜索的速度会变慢，但是不至于用链表或者哈希表那么慢。
    // 我会在内存中用这种树的方式进行存储，在硬盘中只存储一对一对的字符串对，在每次开启数据库的时候先把硬盘上的数据加载到树里。
    // 这是我暂时能想到的最大化节省内存操作时间和硬盘操作时间的方法。
    node* root;
    std::string temp_value;
public:
    database()
    {
        root = new node;
        root->val = 0;
    }
    ~database()
    {

    }
    bool put_db(std::string&, std::string&);
    bool delete_db(std::string&);
    std::string get_db(std::string&);
};

class rw    //用来统一管理数据库文件的读写
{
private:
    FILE* fp;
public:
    rw()
    {
        fp = fopen("dyykvdb.db", "w+");
    }
    void load()
    {

    }
    bool rw_put();
    bool rw_delete();
    // 这里的删除会比put和get复杂得多。
    // 因为按照我的构想，在文件内的数据就只是键-值这样一对一对地简单排放，没有数据结构。
    // 我的想法是，如果有删除命令，先在内存中把那个key的映射删除掉，然后建立一个deletelist文件。
    // 如果是正常的关闭数据库的操作，程序会比对本地数据和deletelist，从本地数据库中删除掉deletelist中存在的键值对，然后删掉deletelist文件。
    // 如果运行过程中被意外重启，那么deletelist文件就不会被删除，这样可以在开启数据库的时候进行上述的删除操作。
    // 因为这种遍历方式是最耗时的，所以我会把它从这里独立出来，缩短两个accept之间的时间。
    std::string rw_get();
};

//向外暴露的c风格的函数，其他代码段可以直接通过这三个函数访问
bool put_f(std::string&, std::string&);
bool delete_f(std::string&);    //直接用delete会和c++的关键字重合
std::string get_f(std::string&);