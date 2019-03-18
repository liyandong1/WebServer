#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "Logger.h"
#include "Config.h"
#include <memory>
#include <functional>

class HttpConnection;
typedef std::function<void(const std::shared_ptr<HttpConnection>&, std::vector<char>&, size_t)> MessageCallback;
typedef std::function<void(const std::shared_ptr<HttpConnection>&)> CloseCallback;
typedef std::function<void(const std::shared_ptr<HttpConnection>&)> ConnectionCallback;

//定义日志宏
#define INFO ConsoleLogger()()
#define INFO_ERR ConsoleLogger()(Level::Error)
#define LOG FileLogger()()
#define LOG_ERR FileLogger()(Level::Error)
#define LOG_FATAL FileLogger()(Level::Fatal)


#endif