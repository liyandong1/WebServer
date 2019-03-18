
//@Athor LiYanDong
//@Email 416620418@qq.com

#include <string.h>

#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <map>
#include "Logger.h"

#ifdef WIN32
#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
#endif


std::string FileLogger::filePath_;

void FileLogger::setFilePath(const std::string& path)
{
	filePath_ = path;
}

static const std::map<Level, const char *> LevelStr =
{
    { Level::Debug, "Debug" },
    { Level::Info, "Info" },
    { Level::Warning, "Warning" },
    { Level::Error, "Error" },
    { Level::Fatal, "Fatal" },
};

std::ostream& operator<< (std::ostream& stream, const tm* tm)
{
    return stream << 1900 + tm->tm_year << '-'
        << std::setfill('0') << std::setw(2) << tm->tm_mon + 1 << '-'
        << std::setfill('0') << std::setw(2) << tm->tm_mday << ' '
        << std::setfill('0') << std::setw(2) << tm->tm_hour << ':'
        << std::setfill('0') << std::setw(2) << tm->tm_min << ':'
        << std::setfill('0') << std::setw(2) << tm->tm_sec;
}

BaseLogger::LogStream BaseLogger::operator()(Level nLevel)
{
    return LogStream(*this, nLevel);
}

const tm* BaseLogger::getLocalTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    localtime_r(&in_time_t, &_localTime);
    return &_localTime;
}

//所有的派生类流对象析构时都调用endline往屏幕输出
//控制这个函数调用,就是让属于当前对象的(多个operator<<连续执行完毕)
void BaseLogger::endline(Level nLevel, std::string&& oMessage)
{
    _lock.lock();
    output(getLocalTime(), LevelStr.find(nLevel)->second, oMessage.c_str());
    _lock.unlock();
}

void ConsoleLogger::output(const tm *p_tm,const char *str_level,const char *str_message)
{
    std::cout << '[' << p_tm << ']'
        << '[' << str_level << "]"
        << "\t" << str_message << std::endl;
    std::cout.flush();
}

FileLogger::FileLogger() noexcept : BaseLogger()
{
	filePath_[filePath_.size() - 1] = '\0';
    _file.open(filePath_, std::fstream::out | std::fstream::app | std::fstream::ate);
    assert(!_file.fail());
}

FileLogger::~FileLogger()
{
    _file.flush();
    _file.close();
}

void FileLogger::output(const tm *p_tm,const char *str_level, const char *str_message)
{
    _file << '[' << p_tm << ']'
        << '[' << str_level << "]"
        << "\t" << str_message << std::endl;
    _file.flush();
}
