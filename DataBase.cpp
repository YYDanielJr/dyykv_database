#include "include/DataBase.h"

extern Logger logger;
database_manager db_manager;

// --------------------内存管理的成员函数------------------------

bool database::put_db(std::string &key, std::string &value)
{
    node *p = root;
    for (int i = 0; i < key.length(); i++)
    {
        bool if_exist = false;
        for (int j = 0; j < p->next.size(); j++)
        {
            if (p->next.at(i)->val == key[i])
            {
                p = p->next.at(i);
                if_exist = true;
                break;
            }
        }
        if (!if_exist)
        {
            node *temp = new node;
            temp->val = key[i];
            p->next.push_back(temp);
            p = temp;
        }
    }
    p->value_node = value;
    return true;
}

bool database::delete_db(std::string &key)
{
    node *p = root;
    for (int i = 0; i < key.length(); i++)
    {
        if (p->next.at(i)->val == key[i])
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

std::string database::get_db(std::string &key)
{
    node *p = root;
    std::string returner;
    for (int i = 0; i < key.length(); i++)
    {
        if (p->next.at(i)->val == key[i])
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

std::vector<std::string> database::traverse()
{
    std::vector<std::string> whole_tree;
    search_node(root, "", whole_tree);
    return whole_tree;
}

void database::search_node(node *p, std::string key, std::vector<std::string> &string_saver)
{
    for (int i = 0; i < p->next.size(); i++)
    {
        search_node(p->next.at(i), key + p->val, string_saver);
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
                  << value << std::endl;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool rw::rw_delete(std::string &key)
{
    try
    {
        deletelist_writer << key << std::endl;
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

void rw::save(std::vector<std::string> &key_value)
{
    db_writer.close();
    db_writer.open("dyykvdb.db", std::ios::out);
    for (auto i = key_value.begin(); i <= key_value.end(); i++)
    {
        db_writer << *i << std::endl;
    }
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
            auto i = temp.begin();
            while (i <= temp.end())
            {
                db->put_db(*i, *(i + 1));
                i += 2;
            }
            logger.info("Local database file loaded.");
        }
        temp.clear();
    }
    catch (std::exception &e)
    {
        logger.error("Exception received when loading local file. A new file will be created.");
    }
    try
    {
        if (!database_file_manager->if_deletelist_null())
        {
            std::vector<std::string> temp = database_file_manager->load_deletelist();
            auto i = temp.begin();
            while (i <= temp.end())
            {
                db->delete_db(*i);
                i++;
            }
            logger.info("Recovered.");
        }
    }
    catch (...)
    {
        logger.error("Exception received when loading exist deletelist.");
    }
    logger.info("Database set up successfully.");
}

void database_manager::shutdown_database_recorder()
{
    std::vector<std::string> temp;
    if (!db)
    {
        temp = db->traverse();
        delete db;
    }
    if (!database_file_manager)
    {
        database_file_manager->save(temp);
        delete database_file_manager;
    }
    system("rm ./deletelist");
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
    bool returner = db->delete_db(key);
    database_file_manager->rw_delete(key);
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