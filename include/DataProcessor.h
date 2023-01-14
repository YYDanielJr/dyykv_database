#pragma once
//这一组文件里会存放解析包的相关代码，并连接到数据库。
#include "Logger.h"
#include "DataBase.h"
#include <string>
#include <mutex>
namespace header_info   //这个namespace里定义一个header结构体和一个package_type的枚举
{
    struct header   //Tcp通信的header
    {
        unsigned int magic_number = 1234;
        unsigned int package_length;
        unsigned int p_type;
        unsigned int zero = 0;
    };

    enum package_type   //包类型
    {
        PUT_REQUEST = 0,
        DELETE_REQUEST,
        GET_REQUEST,
        PUT_RESPONSE,
        DELETE_RESPONSE,
        GET_RESPONSE
    };
}

class PackageResolver   //解析header，同时处理包数据
{
private:
    header_info::header head;
    header_info::header reply_head;
    char raw_header[16];
    char raw_package[1008];
    std::mutex pr_mtx;
public:
    enum processor_exception
    {
        WRONG_MAGIC_NUMBER = 1234,
        UNKNOWN_REQUEST_TYPE = 0
    };  
    void processBuffer(char*);  //传入读到的原始缓冲区
    bool processPutPackage();
    bool processDeletePackage();
    std::string processGetPackage();
    void generateReply(char*);  //生成回复（header+响应包），需要传入一个缓冲区，请将缓冲区的大小固定为1024
};