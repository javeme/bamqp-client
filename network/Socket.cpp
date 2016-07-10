#include "Socket.h"
#include "AceSocket.h"

namespace Network
{

Socket::Socket( EventLoop* loop, NetworkEventHandler* eventHandler )
{
    m_loop = nullptr;
    m_eventHandler = nullptr;

    setEventLoop(loop);   
    setEventHandler(eventHandler);   
}

Socket::~Socket( void )
{
    setEventLoop(nullptr);
    setEventHandler(nullptr);  
}

void Socket::setEventLoop(EventLoop* val)
{
    if(m_loop != nullptr)
        m_loop->unregisterEventHandle(this);

    m_loop = val;

    if(m_loop != nullptr)
        m_loop->registerEventHandle(this);
}

void Socket::setEventHandler(NetworkEventHandler* val)
{
    m_eventHandler = val;
}


SocketFactory::SocketFactory()
{
    ;
}

SocketFactory::~SocketFactory()
{
    ;
}

SocketFactory& SocketFactory::instance()
{
    static SocketFactory factory;
    return factory;
}

Socket* SocketFactory::getSocket(const char* socketType)
{
    if (strcmp(socketType, "TCP-ACE") == 0)
    {
        return new AceSocket();
    }
    else
    {
        return nullptr;
    }
}

void SocketFactory::releaseSocket(Socket* socket)
{
    delete socket;
}

}