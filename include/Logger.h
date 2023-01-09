#pragma once
//这一个头文件用来处理日志。
#include <cstdio>
#include <chrono>
#include <string>
#include <sstream>  //用于字符串转换，在这里转换线程id
#include <thread>
#include <mutex>

std::string getTime();
unsigned long long thread_id_to_ulonglong(std::thread::id&&);

class FileLogger
{
private:
    FILE* logfile;
    unsigned long long threadnum;
    std::recursive_mutex mtx;
public:
    FileLogger()
    {
        logfile = nullptr;
    }
    FileLogger(unsigned long long&& thread_num, const char* filepath)  //构造函数需要提供线程id和文件路径
    {
        threadnum = thread_num;
        logfile = fopen(filepath, "a");
    }
    void relink(unsigned long long thread_num, const char* filepath)  //与有参构造一致
    {
        threadnum = thread_num;
        logfile = fopen(filepath, "a");
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
        std::lock_guard<std::recursive_mutex> guarder(mtx);
        fprintf(logfile, "[ %s ] ", getTime().c_str());
        fprintf(logfile, "*From thread %d : ", threadnum);
        fprintf(logfile, "%s\n", msg);
    }
    void writeError(const char* msg)
    {
        std::lock_guard<std::recursive_mutex> guarder(mtx);
        fprintf(logfile, "[ %s ] ", getTime().c_str());
        fprintf(logfile, "(E)From thread %d : ", threadnum);
        fprintf(logfile, "%s\n", msg);
    }
};

class ScreenLogger
{
private:
    unsigned long long threadnum;
    std::recursive_mutex mtx;
public:
    ScreenLogger() {}
    ScreenLogger(unsigned long long&& thread_num) { threadnum = thread_num; }   //构造函数需要传递线程id
    void relink(unsigned long long thread_num) { threadnum = thread_num; }  //与有参构造一致

    void showInfo(const char* msg)
    {
        std::lock_guard<std::recursive_mutex> guarder(mtx);
        printf("[ %s ] ", getTime().c_str());
        printf("*From thread %d : ", threadnum);
        printf("%s\n", msg);
    }
    void showError(const char* msg)
    {
        std::lock_guard<std::recursive_mutex> guarder(mtx);
        printf("\033[0m\033[1;33m[ %s ]\033[0m ", getTime().c_str());
        printf("\033[0m\033[1;33m(E)From thread %d : \033[0m", threadnum);
        printf("\033[0m\033[1;33m%s\n\033[0m", msg);
    }
};

class Logger    //同时进行文件记录和屏幕输出的类
{
private:
    FileLogger file_logger;
    ScreenLogger screen_logger;
public:
    Logger() {}
    Logger(unsigned long long thread_num, const char* filepath)
    {
        file_logger.relink(thread_num, filepath);
        screen_logger.relink(thread_num);
    }
    void relink(unsigned long long thread_num, const char* filepath)
    {
        file_logger.relink(thread_num, filepath);
        screen_logger.relink(thread_num);
    }
    void info(const char* msg)
    {
        file_logger.writeInfo(msg);
        screen_logger.showInfo(msg);
    }
    void info(std::string& msg)
    {
        file_logger.writeInfo(msg.c_str());
        screen_logger.showInfo(msg.c_str());
    }
    void error(const char* msg)
    {
        file_logger.writeError(msg);
        screen_logger.showError(msg);
    }
    void error(std::string msg)
    {
        file_logger.writeError(msg.c_str());
        screen_logger.showError(msg.c_str());
    }
};