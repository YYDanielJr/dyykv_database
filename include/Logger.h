#pragma once
//这一个头文件用来处理日志。
#include <cstdio>
#include <chrono>
#include <string>
#include <mutex>
#include <fstream>

std::string getTime();

class FileLogger
{
private:
    std::ofstream logfile;
public:
    FileLogger()
    {
        logfile.open("dyykvdb_log.txt", std::ios::out);
    }
    ~FileLogger()
    {
        if(logfile.is_open())
        {
            logfile.close();
        }
    }
    
    void writeInfo(const char* msg)
    {
        logfile << "[ " << getTime().c_str() << " ] " << msg << std::endl;
    }
    void writeError(const char* msg)
    {
        logfile << "[ " << getTime().c_str() << " ] (ERR)" << msg << std::endl;
    }
};

class ScreenLogger
{
public:
    ScreenLogger() {}

    void showInfo(const char* msg)
    {
        printf("[ %s ] ", getTime().c_str());
        printf("%s\n", msg);
    }
    void showError(const char* msg)
    {
        printf("\033[0m\033[1;33m[ %s ]\033[0m ", getTime().c_str());
        printf("\033[0m\033[1;33m%s\n\033[0m", msg);
    }
};

class Logger    //同时进行文件记录和屏幕输出的类
{
private:
    FileLogger file_logger;
    ScreenLogger screen_logger;
    std::mutex mtx;
public:
    Logger() {}
    void info(const char* msg)
    {
        std::lock_guard<std::mutex> guarder(mtx);
        file_logger.writeInfo(msg);
        screen_logger.showInfo(msg);
    }
    void info(std::string& msg)
    {
        std::lock_guard<std::mutex> guarder(mtx);
        file_logger.writeInfo(msg.c_str());
        screen_logger.showInfo(msg.c_str());
    }
    void error(const char* msg)
    {
        std::lock_guard<std::mutex> guarder(mtx);
        file_logger.writeError(msg);
        screen_logger.showError(msg);
    }
    void error(std::string msg)
    {
        std::lock_guard<std::mutex> guarder(mtx);
        file_logger.writeError(msg.c_str());
        screen_logger.showError(msg.c_str());
    }
};