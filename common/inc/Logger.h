//@Athor LiYanDong
//@Email 416620418@qq.com
#ifndef _LOGGER_H_
#define _LOGGER_H_


#include <string>
#include <fstream>
#include <sstream>
#include <mutex>

struct tm;
enum class Level { Debug, Info, Warning, Error, Fatal };
class FileLogger;
class ConsoleLogger;
class BaseLogger;

class BaseLogger
{
    class LogStream;
private:
    std::mutex _lock;
    tm _localTime;
    const tm* getLocalTime();
    void endline(Level nLevel, std::string&& oMessage);
    virtual void output(const tm *p_tm, const char *str_level,const char *str_message) = 0;

public:
    virtual LogStream operator()(Level nLevel = Level::Debug);
    BaseLogger() = default;
    virtual ~BaseLogger() = default;
};


//ostringstream就是有现成的文本缓冲区,重写析构函数
class BaseLogger::LogStream : public std::ostringstream
{
private:
    BaseLogger& m_oLogger;
    Level        m_nLevel;
public:
    LogStream(BaseLogger& oLogger, Level nLevel)
        : m_oLogger(oLogger), m_nLevel(nLevel) {};
    LogStream(const LogStream& ls)
        : m_oLogger(ls.m_oLogger), m_nLevel(ls.m_nLevel) {};
    ~LogStream()
    {
        m_oLogger.endline(m_nLevel, std::move(str()));
    }
};

//写到流对象
class ConsoleLogger : public BaseLogger
{
private:
    using BaseLogger::BaseLogger;
    virtual void output(const tm *p_tm,const char *str_level,const char *str_message);
};


class FileLogger : public BaseLogger
{
private:
	static std::string filePath_;
    std::ofstream _file;
    virtual void output(const tm *p_tm,const char *str_level,const char *str_message);
public:
    FileLogger() noexcept;
    FileLogger(const FileLogger&) = delete;
    FileLogger(FileLogger&&) = delete;
    virtual ~FileLogger();
	
	static void setFilePath(const std::string& path);
};

/*
#define INFO ConsoleLogger()()
#define INFO_ERR ConsoleLogger()(Level::Error)
#define LOG FileLogger()()
#define LOG_ERR FileLogger()(Level::Error)
*/

#endif