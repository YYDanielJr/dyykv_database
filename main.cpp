#include "include/ThreadPool.h"
#include "include/TcpWork.h"
#include "include/Logger.h"
#include <iostream>
#include <mutex>
#include <chrono>

int main()
{
    Connector ctr(AF_INET, SOCK_STREAM, 0, 1);
    ctr.start_connection();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ctr.shutdown_pool();
    return 0;
}