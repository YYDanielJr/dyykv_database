#pragma once
//这一个头文件用来处理日志。
#include <cstdio>
#include <chrono>
#include <string>
#include <mutex>

std::string getTime();

class FileLogger
{
private:
    FILE* logfile;
public:
    FileLogger()
    {
        logfile = fopen("dyykvdb_log.txt", "w");
    }
    ~FileLogger()
    {
        if(logfile)
        {
            fclose(logfile);
        }
    }
    
    void writeInfo(const char* msg)
    {
        fprintf(logfile, "[ %s ] ", getTime().c_str());
        fprintf(logfile, "%s\n", msg);
    }
    void writeError(const char* msg)
    {
        fprintf(logfile, "[ %s ] ", getTime().c_str());
        fprintf(logfile, "(E)%s\n", msg);
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