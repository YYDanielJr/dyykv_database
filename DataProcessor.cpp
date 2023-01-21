#include "include/DataProcessor.h"

extern Logger logger;
extern database_manager db_manager;

void PackageResolver::processBuffer(char* raw_buffer)
{
    memset(raw_header, 0, 16);
    memset(raw_package, 0, 1008);
    //处理header
    for (int i = 0; i < 16; i++)
    {
        raw_header[i] = raw_buffer[i];
    }
    unsigned int* data_p = (unsigned int*)raw_header;
    head.magic_number = *data_p;
    data_p++;
    head.package_length = *data_p;
    data_p++;
    head.p_type = *data_p;
    data_p++;
    head.zero = *data_p;
    //处理数据域
    for (int i = 0; i < head.package_length && i < 1008; i++)
    {
        raw_package[i] = raw_buffer[i + 16];
        //调试代码
        printf("%d ", raw_package[i]);
    }
    //调试代码
    printf("\n%d %d %d %d\n", head.magic_number, head.package_length, head.p_type, head.zero);
}

unsigned int PackageResolver::getPackageSize(char* raw_buffer)
{
    unsigned int* data_p = (unsigned int*)raw_buffer;
    data_p++;
    return *data_p;
}

void PackageResolver::generateReply(char* reply_buffer)
{
    if(head.magic_number != 1234)
    {
        logger.error("Received wrong magic number. Check if the connection is a valid one.");
        std::ostringstream mnum;
        mnum << "Data: " << head.magic_number << ", " << head.package_length << ", " << head.p_type << ", " << head.zero << ".";
        logger.error(mnum.str());
        throw(WRONG_MAGIC_NUMBER);
    }
    bool bool_returner;
    std::string string_returner;
    unsigned int package_size;
    switch(head.p_type)
    {
        case header_info::PUT_REQUEST:
        {
            bool_returner = processPutPackage();
            package_size = 1;
            break;
        }
        case header_info::DELETE_REQUEST:
        {
            bool_returner = processDeletePackage();
            package_size = 1;
            break;
        }
        case header_info::GET_REQUEST:
        {
            string_returner = processGetPackage();
            package_size = 4 + string_returner.length();
            break;
        }
        default:
        {
            logger.error("Unknown request type received from the client.");
            throw(UNKNOWN_REQUEST_TYPE);
        }
    }
    reply_head.magic_number = 1234; //reply的魔数
    reply_head.p_type = head.p_type + 3;    //reply的类型
    reply_head.zero = 0;
    for(int i = 0; i < 1024; i++)
    {
        reply_buffer[i] = 0;    // 初始化reply_buffer
    }
    unsigned int* p = (unsigned int*)reply_buffer;
    *p = reply_head.magic_number;   // 把1234传给reply_buffer
    p++;
    *p = package_size;
    p++;
    *p = reply_head.p_type;
    p++;
    *p = 0;
    p++;
    if(string_returner.size() != 0)
    {
        *p = string_returner.size();
        p++;
        strcat((char*)p, string_returner.c_str());
    }
    else
    {
        *p = bool_returner;
    }
}

bool PackageResolver::processPutPackage()
{
    struct put_package
    {
        unsigned int key_size;
        unsigned int value_size;
        std::string key;
        std::string value;
    } package;
    // 将原先的缓冲区包信息提取并放入package这个结构体中
    unsigned int* p = (unsigned int*)raw_package;
    package.key_size = *p;
    for(int i = 4; i < package.key_size + 4; i++)
    {
        package.key.push_back(raw_package[i]);
    }
    p = (unsigned int*)&raw_package[package.key_size + 4];
    package.value_size = *p;
    for(int i = package.key_size + 8; i < package.key_size + 8 + package.value_size; i++)
    {
        package.value.push_back(raw_package[i]);
    }
    // 接入database
    return db_manager.put_f(package.key, package.value);
}

bool PackageResolver::processDeletePackage()
{
    unsigned int key_size;
    std::string key;
    // 解包
    unsigned int* p = (unsigned int*)raw_package;
    key_size = *p;
    for(int i = 4; i < key_size + 4; i++)
    {
        key.push_back(raw_package[i]);
    }
    // 接入database
    return db_manager.delete_f(key);
}

std::string PackageResolver::processGetPackage()
{
    unsigned int key_size;
    std::string key;
    // 解包
    unsigned int* p = (unsigned int*)raw_package;
    key_size = *p;
    for(int i = 4; i < key_size + 4; i++)
    {
        key.push_back(raw_package[i]);
    }
    // 接入database
    std::string value;
    value = db_manager.get_f(key);
    return value;
}