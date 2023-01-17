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
    fputs(key.c_str(), fp);
    fputc('\n', fp);
    fputs(value.c_str(), fp);
    fputc('\n', fp);
    return true;
}

bool rw::rw_delete(std::string &key)
{
    fputs(key.c_str(), fp_deletelist);
    fputc('\n', fp_deletelist);
    return true;
}

std::vector<std::string> rw::load()
{
    std::vector<std::string> temp;
    while (1)
    {
        char buffer[1024] = {0};
        if (fgets(buffer, 1024, fp) != NULL)
        {
            temp.push_back(buffer);
            memset(buffer, 0, 1024);
        }
        else
        {
            break;
        }
    }
    return temp;
}

std::vector<std::string> rw::load_deletelist()
{
    std::vector<std::string> temp;
    while (1)
    {
        char buffer[1024] = {0};
        if (fgets(buffer, 1024, fp_deletelist) != NULL)
        {
            temp.push_back(buffer);
            memset(buffer, 0, 1024);
        }
        else
        {
            break;
        }
    }
    return temp;
}

void rw::save(std::vector<std::string> &key_value)
{
    FILE *fp_save = fopen("data/savefile", "w");
    for (int i = 0; i < key_value.size(); i++)
    {
        fputs(key_value.at(i).c_str(), fp_save);
        fputc('\n', fp_save);
    }
    fclose(fp_save);
}

// --------------------接口函数--------------------------

void database_manager::start_database_recorder()
{
    db = new database;
    database_file_manager = new rw;
    std::vector<std::string> temp = database_file_manager->load();
    auto i = temp.begin();
    while (i <= temp.end())
    {
        db->put_db(*i, *(i + 1));
        i += 2;
    }
    logger.info("Local database file loaded.");
    temp.clear();
    if (!database_file_manager->if_deletelist_null())
    {
        logger.info("Detected delete list. This may happen if server shut down unexpectedly last time.");
        logger.info("Recovering...");
        temp = database_file_manager->load_deletelist();
        i = temp.begin();
        while (i <= temp.end())
        {
            db->delete_db(*i);
            i++;
        }
        logger.info("Recovered.");
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
    system("cp ./data/savefile ./data/dyykvdb.db");
    system("rm ./data/savefile");
    system("rm ./data/deletelist");
    logger.info("Database shutt down successfully.");
}

bool database_manager::put_f(std::string &key, std::string &value)
{
    logger.info("Put request received.");
    bool returner = db->put_db(key, value);
    database_file_manager->rw_put(key, value);
    return returner;
}

bool database_manager::delete_f(std::string &key)
{
    logger.info("Delete request received.");
    bool returner = db->delete_db(key);
    database_file_manager->rw_delete(key);
    return returner;
}

std::string database_manager::get_f(std::string &key)
{
    logger.info("Get request received.");
    std::string value = db->get_db(key);
    return value;
}