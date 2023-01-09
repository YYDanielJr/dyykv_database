#include "include/DataProcessor.h"

void PackageResolver::processBuffer(char* raw_buffer)
{
    //处理header
    for (int i = 0; i < 16; i++)
    {
        raw_header[i] = raw_buffer[i];
    }
    unsigned int* data_p = (unsigned int*)raw_header;
    head.magic_number = *data_p;
    data_p++;
    head.p_type = *data_p;
    data_p++;
    head.package_length = *data_p;
    data_p++;
    head.zero = *data_p;
    //处理数据域
    for (int i = 0; i < head.package_length; i++)
    {
        raw_package[i] = raw_buffer[i + 16];
    }
}

void PackageResolver::generateReply(char* reply_buffer)
{
    reply_head.magic_number = 1234;
    reply_head.p_type = head.p_type + 3;
    reply_head.zero = 0;
    for(int i = 0; i < 1024; i++)
    {
        reply_buffer[i] = 0;
    }
    unsigned int* p = (unsigned int*)reply_buffer;
    *p = reply_head.magic_number;
    p++;
    
    *p = reply_head.p_type;
    p++;
    *p = 0;
}

void PackageResolver::processPutPackage()
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
    // 
    
}

void PackageResolver::processDeletePackage()
{

}

void PackageResolver::processGetPackage()
{

}