#include "include/ThreadPool.h"
#include "include/TcpWork.h"
#include "include/Logger.h"
#include "include/DataBase.h"
#include "include/DataProcessor.h"
#include <cstdio>
#include <mutex>
#include <chrono>
#include <termio.h> //监听键盘事件

extern database_manager db_manager;
extern Logger logger;
extern Connector connector;

int scanKeyboard()  //通过这个函数实现键盘事件的监听
{
    // Windows底下有_kbhit()，linux竟然没有，有点恼人
    int input;
    struct termios new_settings;
    struct termios stored_settings;
    tcgetattr(0, &stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    // 这里有一个位操作，ICANON是一个整行缓存，~了它的目的是屏蔽整行缓存实现单个读取
    new_settings.c_cc[VTIME] = 0;
    tcgetattr(0, &stored_settings);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);
    // 个人理解，上面的程序段应该是在保证输入流通畅？或者说是在重新开启输入流？
    input = getchar();
    tcsetattr(0, TCSANOW, &stored_settings);
    // 来自csdn：通过tcsetattr函数设置terminal的属性来控制需不需要回车来结束输入。
    return input;
}

void listening_keyboard()
{
    while(1)
    {
        if(scanKeyboard() == 27)    // 如果用户按下esc键
        {
            logger.info("Esc key has been pressed. Shutdown command received.");
            connector.shutdown_pool();  //关闭线程池，组织连接
            db_manager.shutdown_database_recorder();    //关闭数据库管理器，做扫尾操作
            break;
        }
        else
        {
            continue;   //否则继续循环
        }
    }
}

// 主线程的工作流程：
// 在DataBase里有一个全局变量db_manager，打开数据库管理器；
// 在TcpWork里有一个全局变量connector，管理新客户端的连接，但是需要手动管理线程池；
int main()
{
    printf("The server will start soon. Shutdown the server by hitting the Esc key on your keyboard.\n");
    try
    {
        db_manager.start_database_recorder();
    }
    catch(...)
    {

        printf("Database start failed. Exit.\n");
        return 0;
    }
    
    try
    {
        std::thread accept_thread(&Connector::start_connection, &connector);
        accept_thread.detach();
    }
    catch(const int& connection_exception_type)
    {
        if(connection_exception_type == Connector::LISTENING_STATUS_ERROR)
        {
            logger.error("Listening is not properly set. Ckeck if all listening settings are correct. No application is using port 11451.");
            printf("Exit.\n");
            return 0;
        }
        if(connection_exception_type == Connector::ACCEPT_ERROR)
        {
            logger.error("Acception to a new client failed.");
        }
    }
    std::thread keyevent_thread(listening_keyboard);    //给键盘监听事件专门分配一个线程
    keyevent_thread.join();
    printf("Exit.");
    return 0;
}