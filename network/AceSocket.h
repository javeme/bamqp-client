#pragma once
#include "ace/Reactor.h"
#include "ace/Svc_Handler.h"
#include "ace/Acceptor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "Socket.h"

namespace Network
{
    
const static size_t MAX_BUF_SIZE = 1024 * 64;

class AceSocket : public ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH>, public TcpSocket
{
public:
    typedef ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH> SvcHandler;
    typedef ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH> ParentHandler;

    //Add our own Reactor singleton
    typedef ACE_Singleton<ACE_Reactor, ACE_Null_Mutex> Reactor;
    //Create an Acceptor
    typedef ACE_Acceptor<SvcHandler, ACE_SOCK_ACCEPTOR> Acceptor;
    //Create a Connector
    typedef ACE_Connector<SvcHandler, ACE_SOCK_CONNECTOR> Connector;

public:
    AceSocket();
    AceSocket(EventLoop* loop, NetworkEventHandler* eventHandler);
    virtual ~AceSocket(void);

public:
    virtual int connect(const char hostName[], unsigned short port);
    virtual int close();
    virtual size_t send(const char *buffer, size_t size);
    virtual int handleEvents();

public:
    virtual int open(void*);
    virtual int handle_input(ACE_HANDLE handle);
    virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask);
    virtual int handle_timeout(const ACE_Time_Value &time, const void *act = 0);
    virtual int handle_exception(ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual int handle_signal(int signum, siginfo_t * siginfo = 0, ucontext_t * ctx = 0);
    virtual int handle_qos(ACE_HANDLE = ACE_INVALID_HANDLE);

public:
    static ACE_Reactor* getDefaultReactor();

private:
    FixOutBuffer<MAX_BUF_SIZE> m_buffer;
};


/*
//Global stream identifier used by both threads
ACE_SOCK_Stream * MyServiceHandler::Peer=0;
void main_accept()
{
    ACE_INET_Addr addr(PORT_NO);
    Acceptor myacceptor(addr,Reactor::instance());
    while(1)
        Reactor::instance()->handle_events();
    return 0;
}
void main_connect()
{
    ACE_INET_Addr addr(PORT_NO,HOSTNAME);
    Connector myconnector;
    myconnector.connect(my_svc_handler,addr);
    while(1)
        Reactor::instance()->handle_events();
}*/

}
