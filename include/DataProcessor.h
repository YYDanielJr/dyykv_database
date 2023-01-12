#pragma once
//这一组文件里会存放解析包的相关代码，并连接到数据库。
#include "Logger.h"
#include "DataBase.h"
#include <string>
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
public:
    enum processor_exception
    {
        WRONG_MAGIC_NUMBER = 1234,
        UNKNOWN_REQUEST_TYPE = 0
    };  
    void processBuffer(char*);  //传入读到的原始缓冲区
    void processHead()
    {
        if(head.magic_number != 1234)
        {
            logger.error("Received wrong magic number. Check if the connection is a valid one.");
            throw(WRONG_MAGIC_NUMBER);
        }
        switch(head.p_type)
        {
            case header_info::PUT_REQUEST:
            {
                bool returner = processPutPackage();

                break;
            }
            case header_info::DELETE_REQUEST:
            {
                bool returner = processDeletePackage();
                break;
            }
            case header_info::GET_REQUEST:
            {
                std::string returner = processGetPackage();
                break;
            }
            default:
            {
                logger.error("Unknown request type received from the client.");
                throw(UNKNOWN_REQUEST_TYPE);
            }
        }
    }
    bool processPutPackage();
    bool processDeletePackage();
    std::string processGetPackage();
    void generateReply(char*);  //生成回复（header+响应包），需要传入一个缓冲区，请将缓冲区的大小固定为1024
};