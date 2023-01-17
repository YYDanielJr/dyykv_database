#include "include/Logger.h"

Logger logger;

std::string getTime()   //获取系统当前时间，并存储在string里
{
	std::chrono::system_clock::time_point tp = std::chrono::system_clock().now();
	time_t tm = std::chrono::system_clock::to_time_t(tp);
	std::string ts = ctime(&tm);
	ts.resize(ts.size() - 1);   //移除换行符
    return ts;
}
