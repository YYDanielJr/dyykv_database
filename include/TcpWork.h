#pragma once
// 这一组文件里会存放网络通信的相关操作。
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <cstdio>
#include <mutex>
#include <sstream>
#include "ThreadPool.h"
#include "Logger.h"
#include "DataProcessor.h"

namespace tcpwork
{
    struct client_info  //使用结构体来存储多个客户端的信息（通信文件标识符和客户端地址）
    {
        int sock_for_connection; // 连接套接字的文件标识符
        struct sockaddr_in client_addr;
    };
    bool pool_is_running;
}

class Connector
{
private:
    int sock_for_listening; // 监听套接字的文件标识符
    bool if_listening_open;
    bool if_connection_open;
    int max_link;             // 最大连接数
    ThreadPool *default_pool; // 开辟在堆区的线程池，方便手动收回
    struct sockaddr_in addr;
    Logger logger;
    std::mutex mtx;

public:
    enum connect_exception // 异常信息
    {
        SOCKET_CREATION_ERROR,
        BIND_ERROR,
        LISTENING_SETTING_ERROR,
        LISTENING_STATUS_ERROR,
        POOL_IS_NOT_RUNNING,
        ACCEPT_ERROR
    };
    Connector() = delete;
    Connector(const Connector &obj) = delete;
    Connector(int domain, int type, int protocol, int max_link_in); // 默认构造和拷贝构造被禁用。有参构造传入值是socket类型和最大连接数。

    void start_pool(int maxlink)
    {
        default_pool = new ThreadPool(maxlink);
        tcpwork::pool_is_running = true;
    }
    void shutdown_pool()
    {
        if (!tcpwork::pool_is_running)
        {
            throw(POOL_IS_NOT_RUNNING);
        }
        else
        {
            tcpwork::pool_is_running = false;
            delete default_pool;
        }
    }

    void start_connection();
};

void client_resolver(tcpwork::client_info *); // 用于加载到线程池里的全局函数