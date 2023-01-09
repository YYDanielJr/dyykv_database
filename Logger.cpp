#include "include/Logger.h"

std::string getTime()   //获取系统当前时间，并存储在string里
{
	std::chrono::system_clock::time_point tp = std::chrono::system_clock().now();
	time_t tm = std::chrono::system_clock::to_time_t(tp);
	std::string ts = ctime(&tm);
	ts.resize(ts.size() - 1);   //移除换行符
    return ts;
}

unsigned long long thread_id_to_ulonglong(std::thread::id&& obj)    //get_id函数默认返回thread::id对象，将这个对象转换为ulonglong
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    unsigned long long tid = std::stoull(stid);
    return tid;
}
