
#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include "noncopyable.h"
#include "HttpDataParser.h"
#include "Global.h"

using namespace std::placeholders;

class Channel;
class Socket;
class EventLoop;
struct InetAddress;
class HttpConnection;
class EventLoopThreadPool;

class HttpServer:public noncopyable
{
private:
	EventLoop* loop_;
	std::unique_ptr<Socket> acceptSocket_;
	int listenFd;
	int nextCondId_;
	std::unique_ptr<Channel> acceptChannel_;
	bool listening_;
	std::unique_ptr<EventLoopThreadPool> threadPool_;
	
	std::vector<std::shared_ptr<Channel>> httpClientChannels_;
	std::map<std::string, std::shared_ptr<HttpConnection>> connectionMap_;
	
	//HTTP解析器
	HttpDataParser Parser_;
	
	void handleRead();
	void onNewConnection(int fd, const InetAddress& peerAddr);
	

	void removeConnection(const std::shared_ptr<HttpConnection>& conn);
    void removeConnectionInLoop(const std::shared_ptr<HttpConnection>& conn);	
	
	void onMessage(const std::shared_ptr<HttpConnection>& coon, std::vector<char>& buf, size_t len);
public:
	HttpServer(EventLoop* loop, int listenPort, int threadNum);
	void listen();
	bool listening() const;
	~HttpServer();
};

#endif