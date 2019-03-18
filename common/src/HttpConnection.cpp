
#include <unistd.h>
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "HttpConnection.h"

#include <iostream>

HttpConnection::HttpConnection(EventLoop* loop, const std::string& name, int connfd):
				loop_(loop),clientChannel_(new Channel(loop, connfd)),
				clientSocket_(new Socket(std::to_string(connfd))),name_(name)
{
	clientChannel_->setReadCallback(std::bind(&HttpConnection::handleRead, this));
	clientChannel_->setWriteCallback(std::bind(&HttpConnection::handleWrite, this));
	clientChannel_->setErrorCallback(std::bind(&HttpConnection::handleClose, this));
	clientChannel_->setCloseCallback(std::bind(&HttpConnection::handleError, this));
	
	clientSocket_->setKeepAlive(true);
	//loop_->queueInLoop(clientChannel_->enableReading());
	inputBuffer_ = std::vector<char>(2048, '0');
	
}


//根据read的返回值进行回调
void HttpConnection::handleRead()
{
	int n = ::read(clientChannel_->fd(), &*inputBuffer_.begin(), inputBuffer_.size());
    if(n > 0)
    {
		//调用Server设置的函数
		//处理信息
		messageCallback_(shared_from_this(), inputBuffer_, static_cast<size_t>(n));
		
		//注册事件
		
		clientChannel_->enableReading();
    }
    else if(n == 0)
    {
        handleClose();  //回调关闭连接
    }
    else
    {
        handleError();
    }
}

void HttpConnection::handleWrite()
{
	if(outputBuffer_.size() > 0)
	{
		int n = ::write(clientChannel_->fd(), &*outputBuffer_.begin(), outputBuffer_.size());
		if(n < 0)
		{
			perror("Write error...");
		}
	}
	clientChannel_->enableReading();
}

//应该去Server删除当前对象
void HttpConnection::handleClose()
{
	loop_->assertInLoopThread();
    LOG << "TcpConnection::handleClose name = " << name_ << std::endl;
    //不关闭文件描述符，等到析构
    clientChannel_->disableAll();
    closeCallback_(shared_from_this()); //closeCallback_来自TcpServer
}

void HttpConnection::handleError()
{
	int err = clientSocket_->getSocketError();
    LOG << "TcpConnection::handleError [" << name_
        << "] - SO_ERROR =" << err << std::endl;
}

void HttpConnection::setMessageCallback(const MessageCallback& cb)
{
	messageCallback_ = cb;
}

void HttpConnection::setCloseCallback(const CloseCallback& cb)
{
	closeCallback_ = cb;
}

void HttpConnection::setConnectionCallback(const ConnectionCallback& cb)
{
	connectionCallback_ = cb;
}

EventLoop* HttpConnection::getLoop()
{
	return loop_;
}

void HttpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	clientChannel_->tie(shared_from_this());
	clientChannel_->enableReading();
}

void HttpConnection::connectDestory()
{
	loop_->assertInLoopThread();
	clientChannel_->disableAll();
}

void HttpConnection::setReturnMessage(const std::vector<char>& message)
{
	outputBuffer_.clear();
	outputBuffer_ = message;
	
	//这个函数里面再使能写事件
	clientChannel_->enableWriting();
}

HttpConnection::~HttpConnection()
{
	LOG << "Http Client[" << name_ << "] dtor" << std::endl;
}