#include "include/DataBase.h"

extern Logger logger;
database_manager db_manager;
std::mutex db_mutex;

// --------------------内存管理的成员函数------------------------

bool database::put_db(std::string &key, std::string &value)
{
    //std::lock_guard<std::mutex> guarder(db_mutex);
    try
    {
        db_mutex.lock();
        node *p = root;
        db_mutex.unlock();
        for (int i = 0; i < key.length(); i++) // 第一层循环，循环key的长度次
        {
            bool if_exist = false;
            for (int j = 0; j < p->next.size(); j++) // 第二层循环，循环当前孩子个数次，这层循环的目的是寻找这一层是否已经存在了目标的孩子
            {
                if (p->next.at(j)->val == key[i]) // 如果第j个孩子是key的第i个字母
                {
                    p = p->next.at(j); // 就把p指针向第j个孩子那里移动
                    if_exist = true;
                    break;
                }
            }
            if (!if_exist) // 如果这个孩子不存在
            {
                node *temp = new node;   // 就新建结点
                temp->val = key[i];      // 这个结点的值是key的第i个字母
                p->next.push_back(temp); // 这个结点的指针推到p的孩子池里
                p = temp;   //把p指针向新建的结点移动
            }
        }
        // 循环了key的长度次之后，退出循环，p是这个key的最后一个字母
        p->value_node = value; // 把value给最后一个字母，用每个字母长度的value_node长度是否为0来判断是否有对应的value
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool database::delete_db(std::string &key)
{
    //std::lock_guard<std::mutex> guarder(db_mutex);
    db_mutex.lock();
    node *p = root;
    db_mutex.unlock();
    for (int i = 0; i < key.length(); i++)
    {
        bool if_found_child = false;
        for(int j = 0; j < p->next.size(); j++)
        {
            if(p->next.at(j)->val == key[i])
            {
                if_found_child = true;
                p = p->next.at(j);
                break;
            }
        }
        if(!if_found_child)
        {
            return false;
        }
    }
    if(p->value_node.size() != 0)
    {
        p->value_node.clear();
        return true;
    }
    else
    {
        return false;
    }
}

std::string database::get_db(std::string &key)
{
    std::string returner;   //一个空的string，如果在搜索过程中间确定不存在就返回这个空的string
    //std::lock_guard<std::mutex> guarder(db_mutex);
    db_mutex.lock();
    node *p = root;
    db_mutex.unlock();
    for (int i = 0; i < key.length(); i++)
    {
        bool if_found_child = false;
        for(int j = 0; j < p->next.size(); j++)
        {
            if(p->next.at(j)->val == key[i])
            {
                if_found_child = true;
                p = p->next.at(j);
                break;
            }
        }
        if(!if_found_child)
        {
            return returner;
        }
    }
    return p->value_node;
}

std::vector<std::string> database::traverse()
{
    std::vector<std::string> whole_tree;
    std::string word_root;
    search_node(root, word_root, whole_tree);
    //测试代码：
    //----------------------------------------------
    printf("Whole_tree:\n");
    for(int i = 0; i < whole_tree.size(); i++)
    {
        printf("%s\n", whole_tree.at(i).c_str());
    }
    //----------------------------------------------
    return whole_tree;
}

void database::search_node(node *p, std::string &key, std::vector<std::string> &string_saver)
{
    for (int i = 0; i < p->next.size(); i++)
    {
        key.push_back(p->next.at(i)->val);
        search_node(p->next.at(i), key, string_saver);
        key.pop_back();
    }
    if (p->value_node.length() != 0)
    {
        string_saver.push_back(key);
        string_saver.push_back(p->value_node);
    }
}
// --------------------数据库本地文件管理相关函数--------------------

bool rw::rw_put(std::string &key, std::string &value)
{
    try
    {
        db_writer << key << std::endl
                  << value << std::endl
                  << "1" << std::endl;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool rw::rw_delete(std::string &key, std::string &value)
{
    try
    {
        db_writer << key << std::endl << value << std::endl << "0" << std::endl;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

std::vector<std::string> rw::load()
{
    std::vector<std::string> temp;
    char c;
    db_reader >> c;
    if (db_reader.eof())
    {
        return temp;
    }
    db_reader.seekg(std::ios::beg);
    while (!db_reader.eof())
    {
        char buf[1024] = {0};
        db_reader.getline(buf, 1024);
        if (buf[0] != 0 && buf[0] != '\n')
        {
            temp.push_back(buf);
        }
    }
    return temp;
}

/*
std::vector<std::string> rw::load_deletelist()
{
    std::vector<std::string> temp;
    char c;
    deletelist_reader >> c;
    if (deletelist_reader.eof())
    {
        return temp;
    }
    deletelist_reader.seekg(std::ios::beg);
    while (!deletelist_reader.eof())
    {
        char buf[1024] = {0};
        deletelist_reader.getline(buf, 1024);
        if (buf[0] != 0 && buf[0] != '\n')
        {
            temp.push_back(buf);
        }
    }
    return temp;
}
*/

void rw::save(std::vector<std::string> &key_value)
{
    std::ofstream saver("savefile", std::ios::out);
    int count = 0;
    for(int i = 0; i < key_value.size(); i++)
    {
        saver << key_value[i] << std::endl;
        count++;
        if(count == 2)
        {
            saver << "1" << std::endl;
            count = 0;
        }
    }
    saver.close();
}

// --------------------接口函数--------------------------

void database_manager::start_database_recorder()
{
    db = new database;
    database_file_manager = new rw;
    try
    {
        std::vector<std::string> temp = database_file_manager->load();
        if (temp.size() > 0)
        {
            for(int i = 0; i < temp.size(); i += 3)
            {
                if(i + 1 < temp.size() && (temp[i][0] >= 32 && temp[i][0] <= 126))  //避免数组越界或者读入空字符
                {
                    if(temp[i + 2] == "1")
                    {
                        db->put_db(temp[i], temp[i + 1]);
                    }
                    else if(temp[i + 2] == "0")
                    {
                        db->delete_db(temp[i]);
                    }
                    else
                    {
                        throw(0);   //0代表db文件出现问题
                    }
                }
            }
            logger.info("Local database file loaded.");
        }
        temp.clear();
    }
    catch (std::exception &e)
    {
        logger.error("Exception received when loading local file. A new file will be created.");
    }
    /*
    try
    {
        if (!database_file_manager->if_deletelist_null())
        {
            std::vector<std::string> temp = database_file_manager->load_deletelist();
            //测试代码：
            //-----------------------------------------
            printf("Deletelist:\n");
            for(int i = 0; i < temp.size(); i++)
            {
                printf("%s\n", temp.at(i).c_str());
            }
            //-----------------------------------------
            //测试结果：可以成功读到deletelist
            for(int i = 0; i < temp.size(); i++)
            {
                if(temp[i][0] >= 32 && temp[i][0] <= 126)  //避免数组越界或者读入空字符
                {
                    db->delete_db(temp[i]);
                }
            }
            logger.info("Recovered.");
        }
    }
    catch (...)
    {
        logger.error("Exception received when loading exist deletelist.");
    }
    */
    logger.info("Database set up successfully.");
}

void database_manager::shutdown_database_recorder()
{
    std::vector<std::string> temp;
    if (db)
    {
        temp = db->traverse();
        delete db;
    }
    //测试代码
    //--------------------------------------------------------------
    printf("Traversed Key-value: \n");
    for(int i = 0; i < temp.size(); i += 2)
    {
        printf("%s, %s\n", temp.at(i).c_str(), temp.at(i + 1).c_str());
    }
    printf("Traverse has been put out.\n");
    //--------------------------------------------------------------

    if (database_file_manager)
    {
        database_file_manager->save(temp);
        delete database_file_manager;
    }
    //system("rm ./deletelist");
    system("cp savefile dyykvdb.db");
    system("rm savefile");
    logger.info("Database shut down successfully.");
}

bool database_manager::put_f(std::string &key, std::string &value)
{
    std::ostringstream lg;
    lg << "Put request received. Key is " << key << ", value is " << value << ".";
    std::string temp = lg.str();
    logger.info(temp);
    bool returner = db->put_db(key, value);
    database_file_manager->rw_put(key, value);
    return returner;
}

bool database_manager::delete_f(std::string &key)
{
    std::ostringstream lg;
    lg << "Delete request received. Key is " << key << ".";
    std::string temp = lg.str();
    logger.info(temp);
    temp.clear();
    temp = db->get_db(key);
    bool returner = db->delete_db(key);
    database_file_manager->rw_delete(key, temp);
    return returner;
}

std::string database_manager::get_f(std::string &key)
{
    std::ostringstream lg;
    lg << "Get request received. Key is " << key << ".";
    std::string temp = lg.str();
    logger.info(temp);
    std::string value = db->get_db(key);
    return value;
}