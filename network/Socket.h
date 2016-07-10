#pragma once
#include "NetworkEventHandler.h"
#include "EventLoop.h"

namespace Network
{

class Socket : public EventHandle
{
public:
    Socket(EventLoop* loop, NetworkEventHandler* eventHandler);
    virtual ~Socket(void);
public:
    virtual size_t send(const char *buffer, size_t size) = 0;
    virtual int handleEvents() = 0;
public:
    void setEventLoop(EventLoop* val);
    void setEventHandler(NetworkEventHandler* val);
protected:
    EventLoop* m_loop;
    NetworkEventHandler* m_eventHandler;
};

class TcpSocket : public Socket
{
public:
    TcpSocket(EventLoop* loop, NetworkEventHandler* eventHandler) : Socket(loop, eventHandler) {}
    virtual ~TcpSocket() {}
public:
    virtual int connect(const char hostName[], unsigned short port) = 0;
    virtual int close() = 0;
};

class SocketFactory
{
public:
    static SocketFactory& instance();

    virtual Socket* getSocket(const char* socketType);
    virtual void releaseSocket(Socket* socket);
private:
    SocketFactory();
    virtual ~SocketFactory();
};

}