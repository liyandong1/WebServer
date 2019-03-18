
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <string>
#include "noncopyable.h"

class EventLoop;

struct InetAddress
{
    std::string ip_;
    int port_;
};

class Socket:public noncopyable
{
private:
	const int fd_;    
    int port_;
public:
	explicit Socket(int port);
	explicit Socket(const std::string& fd);
	int fd() const;
	void bindAddress(const std::string& addr = "");
	void listen();
	int accept(InetAddress* peeraddr);
	void shutdownWrite();
	void setReuseAddr(bool on);
	void setReusePort(bool on);
    void setKeepAlive(bool on);
    int getSocketError();
    ~Socket();
};

#endif