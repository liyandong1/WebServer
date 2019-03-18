
#ifndef _HTTPCONNECTION_H_
#define _HTTPCONNECTION_H_

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "noncopyable.h"
#include "Global.h"
//这个客户端是属于哪一个事件循环对象

class Socket;
class Channel;
class EventLoop;


class HttpConnection:public noncopyable,
			public std::enable_shared_from_this<HttpConnection>
{
private:
	EventLoop* loop_;
	std::unique_ptr<Channel> clientChannel_;
	std::unique_ptr<Socket> clientSocket_;
	std::string name_;
	std::vector<char> inputBuffer_;
	std::vector<char> outputBuffer_;
	
	MessageCallback messageCallback_;
	CloseCallback closeCallback_;
	ConnectionCallback connectionCallback_;
	
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();
public:
	HttpConnection(EventLoop* loop, const std::string& name, int connfd);
	void setMessageCallback(const MessageCallback& cb);
	void setCloseCallback(const CloseCallback& cb);
	void setConnectionCallback(const ConnectionCallback& cb);
	EventLoop* getLoop();
	void connectEstablished();
	void connectDestory();
	void setReturnMessage(const std::vector<char>& message);
	std::string name()
	{
		return name_;
	}
	~HttpConnection();
};

#endif