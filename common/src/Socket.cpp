
#include <cstdlib>
#include <cstdio>
#include <assert.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream> ///////

#include "Global.h"
#include "Socket.h"

//epoll+非阻塞io  不是马上accept，而是readable才accept
int createNonblockSocket()
{
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    //int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if(fd < 0)
    {
        LOG_ERR << "createNonblockSocket() error..." << std::endl;
        abort();
    }
    return fd;
}

Socket::Socket(int port):port_(port),fd_(createNonblockSocket())
{
	assert(!(fd_ < 0));
}

Socket::Socket(const std::string& fd):fd_(::atoi(fd.c_str()))
{
}

int Socket::fd() const
{
	return fd_;
}

void Socket::bindAddress(const std::string& addr)
{
	struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    if(addr == "")
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
    {
        if(::inet_pton(AF_INET, addr.c_str(), &serveraddr.sin_addr) < 0)
        {
            LOG_ERR << "bindAddress inet_pton() error..." << std::endl;
            abort();
        }
    }
    serveraddr.sin_port = htons(port_);
    if(::bind(fd_, reinterpret_cast<struct sockaddr*>(&serveraddr), sizeof(serveraddr))<0)
    {
        perror("bind error ...");
        abort();
    }
}

void Socket::listen()
{
	if(::listen(fd_, SOMAXCONN) < 0){
        perror("listen error ...");
        abort();
    }
	
	//std::cout << "Socket::listen()" << std::endl;
}

int Socket::accept(InetAddress* peeraddr)
{
	struct sockaddr_in clientaddr;
    memset(&clientaddr, 0, sizeof(clientaddr));
    int addrlen = sizeof(clientaddr);
    int fd = ::accept4(fd_, reinterpret_cast<struct sockaddr*>(&clientaddr), \
                            reinterpret_cast<socklen_t*>(&addrlen), SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(fd < 0)
    {
        perror("Socket::accept error...");
        abort();
    }
    peeraddr->port_ = ntohs(clientaddr.sin_port);
    char ip[24] = {0};
    if(inet_ntop(AF_INET, reinterpret_cast<void*>(&clientaddr.sin_addr), \
                                        ip, static_cast<socklen_t>(sizeof(ip))) < 0)
    {
        perror("inet_ntop error...");
        abort();
    }
    peeraddr->ip_ = std::string(ip);
    return fd;
}

void Socket::shutdownWrite()
{
	if(::shutdown(fd_, SHUT_WR) < 0)
    {
        perror("shutdown error...");
        abort();
    } 
}

void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on)
{
	int optval = on ? 1 : 0;
    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_ERR << "SO_REUSEPORT failed." << std::endl;
    }
}

void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::getSocketError()
{
	int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        return errno;
    else
        return optval;
}

Socket::~Socket()
{
	::close(fd_);
}