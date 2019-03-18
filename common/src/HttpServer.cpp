
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "Channel.h"
#include "Socket.h"
#include "Global.h"
#include "EventLoop.h"
#include "HttpServer.h"
#include "HttpConnection.h"
#include "EventLoopThreadPool.h"


extern Config* m_config;


HttpServer::HttpServer(EventLoop* loop, int listenPort, int threadNum):loop_(loop),\
		acceptSocket_(new Socket(listenPort)),listenFd(acceptSocket_->fd()),nextCondId_(0),
		acceptChannel_(new Channel(loop, acceptSocket_->fd())),listening_(false),
		threadPool_(new EventLoopThreadPool(loop, threadNum))
{
	//绑定ip
	acceptSocket_->bindAddress(m_config->getStr("IP"));
	acceptSocket_->setReuseAddr(true);
	acceptSocket_->setReusePort(true);
	acceptChannel_->setReadCallback(std::bind(&HttpServer::handleRead, this));
	INFO << "ListenPort " << listenPort << std::endl;
	INFO << "threadNum " << threadNum << std::endl;
	INFO << "IP " << m_config->getStr("IP") << std::endl;
	//启动线程池
	threadPool_->start();  
}

void HttpServer::listen()
{
	//std::cout << "HttpServer::listen()" << std::endl;
	loop_->assertInLoopThread();
	listening_ = true;
	acceptSocket_->listen();
	acceptChannel_->enableReading();
}

bool HttpServer::listening() const
{
	return listening_;
}


//当监听socket可读,也就是有新的客户端连接上来
void HttpServer::handleRead()
{
	loop_->assertInLoopThread();
	//这个位置可以给线程分派任务了
	InetAddress peerAddr;
	int connfd = acceptSocket_->accept(&peerAddr);
	
	if(connfd >= 0)
	{
		onNewConnection(connfd, peerAddr);
	}
	else
	{
		::close(connfd);
	}
}

//处理HTTP消息
void HttpServer::onMessage(const std::shared_ptr<HttpConnection>& coon,\
											std::vector<char>& buf, size_t len)
{
	std::string text = "";
	for(size_t i = 0; i <len; ++i)
		text += buf[i];
	std::cout << text;
	Parser_.process(text);
	std::vector<char> ret = Parser_.getResult();
	//把得到的消息设置到每个客户端缓冲区
	coon->setReturnMessage(ret);
	//for(int i = 0; i < ret.size(); i++)
		//std::cout << ret[i] ;
}

void HttpServer::onNewConnection(int fd, const InetAddress& peerAddr)
{
	loop_->assertInLoopThread();
	std::string connName = std::to_string(nextCondId_);
	LOG << "HttpServer::newConnection [" << connName
        << "] from " << peerAddr.ip_ << " " << peerAddr.port_
        << std::endl;
	//从线程池中选择线程
	EventLoop* ioLoop = threadPool_->getNextLoop();
	//创建连接对象
	std::shared_ptr<HttpConnection> conn(new HttpConnection(ioLoop, connName, fd));
	connectionMap_[connName] = conn;
	
	//conn->setMessageCallback(messageCallback_);
	conn->setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
	conn->setCloseCallback(std::bind(&HttpServer::removeConnection, this, conn));
	//一定要放到io线程调用
	ioLoop->runInLoop(std::bind(&HttpConnection::connectEstablished, conn));
}

void HttpServer::removeConnection(const std::shared_ptr<HttpConnection>& conn)
{
	loop_->runInLoop(std::bind(&HttpServer::removeConnectionInLoop, this, conn));
}

void HttpServer::removeConnectionInLoop(const std::shared_ptr<HttpConnection>& conn)
{
	loop_->assertInLoopThread();
	LOG << "HttpServer::removeConnection [ " << conn->name() << \
						" ] - connection " << std::endl;
	connectionMap_.erase(conn->name());
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&HttpConnection::connectDestory, conn));
}

HttpServer::~HttpServer()
{
	::close(listenFd);
}