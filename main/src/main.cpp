
#include "Global.h"
#include "Logger.h"
#include "Process.h"

#include "Channel.h"
#include "EventLoop.h"
#include "TimerQueue.h"
#include "HttpServer.h"
#include <iostream>

#include <unistd.h>
#include <strings.h>
#include <stdlib.h>


Config* m_config(Config::getInstance());

int lanuchWithDeamon()
{
    //以守护进程方式启动
	std::cout << "m_config->getNum" << m_config->getNum("Daemon") << std::endl;
    if(m_config->getNum("Daemon") == 1)
    {
        int cdaemonresult = Process().create_daemon();
        if(cdaemonresult == -1)//创建失败
        {
            LOG << errno << " " << "Process create daemon error" << std::endl;
            return -1;
        }
        else if(cdaemonresult == 1)//父进程退出
        {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char* argv[])
{
	//装载配置信息
	Config::getInstance()->load("//mnt//hgfs//sharefile//WebServer//WebServer.conf");
	FileLogger::setFilePath(Config::getInstance()->getStr("LogPath"));
	//以守护进程方式启动
    if(lanuchWithDeamon()){
		//设置进程标题
        Process().set_proc_name(argv[0], m_config->getStr("Name"));
	}
    
	EventLoop loop;
	
	HttpServer server(&loop, ::atoi(m_config->getStr("ListenPort").c_str()),\
			::atoi(m_config->getStr("ThreadNum").c_str()));
	server.listen();
	
	loop.loop();
	
	return 0;
}