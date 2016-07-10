#include "AceSocket.h"

namespace Network
{


AceSocket::AceSocket() : TcpSocket(nullptr, nullptr)
{
    ACE_TRACE ("AceSocket::AceSocket()");
    ;
}

AceSocket::AceSocket(EventLoop* loop, NetworkEventHandler* eventHandler) : TcpSocket(loop, eventHandler)
{
    ACE_TRACE ("AceSocket::AceSocket(EventLoop* loop, NetworkEventHandler* eventHandler)");
    ;
}

AceSocket::~AceSocket(void)
{
    ACE_TRACE ("AceSocket::~AceSocket(void)");

    /* ~ACE_Svc_Handler() will do it: 
    (void)reactor()->remove_handler(this, ACE_Event_Handler::ALL_EVENTS_MASK|DONT_CALL);
    */
}

int AceSocket::connect(const char hostName[], unsigned short port)
{
    ACE_TRACE ("AceSocket::connect(const char hostName[], unsigned short port)");

    ACE_INET_Addr addr(port, hostName);//addr.set(port,hostName);
    SvcHandler* svcHandler = this;//const_cast<AceSocket*>(this);
    Connector connector(getDefaultReactor());
    return connector.connect(svcHandler, addr);

    //ACE_SOCK_Connector sockConnector;    
    //return sockConnector.connect(svcHandler->peer(), addr, &ACE_Time_Value(5));
}

int AceSocket::close()
{
    ACE_TRACE ("AceSocket::close()");

    return peer().close();
}

size_t AceSocket::send(const char *buffer, size_t size)
{
    ACE_TRACE ("AceSocket::send(const char *buffer, size_t size)");

    //Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);
    return peer().send_n(buffer, size);
}

int AceSocket::handleEvents()
{
    ACE_Time_Value maxWaitTime(0, 1 * 1000);//1ms
    if(reactor())
    {
        int ret = reactor()->handle_events(maxWaitTime);
        //printf(">>>>>>>>>> handle_events:  ret = %d, error = %s \n", ret, strerror(errno));
        return ret;
    }

    return 0;
}

int AceSocket::open(void* para)
{
    ACE_TRACE ("AceSocket::open(void* para)");

    if (m_eventHandler != nullptr)
    {
        m_eventHandler->onConnected(this);
    }

    //Register with the reactor to remember this handle
    if(reactor())
    {
        int ret = reactor()->register_handler(this, ACE_Event_Handler::READ_MASK);//EXCEPT_MASK
        //printf(">>>>>>>>>> register_handler:  ret = %d \n", ret);
        if(ret == -1)
            return -1;
    }

    //keep the service handler registered by returning 0 to the reactor
    return ParentHandler::open(para);
}

int AceSocket::handle_input(ACE_HANDLE handle)
{
    ACE_TRACE ("AceSocket::handle_input(ACE_HANDLE handle)");

    size_t remain = m_buffer.remains();
    char* data = (char*)m_buffer.current();

    //buffer is not full
    if(remain > 0)
    {
        ssize_t recvedSize = peer().recv(data, remain);//recv_n
        
        //Check if peer aborted the connection
        if(recvedSize < 0)
        {
            if(errno == EINTR)/* interrupted by sig handler , call read() again */
            
            if(m_eventHandler != nullptr)
            {
                m_eventHandler->onFailure(this);
            }
            return -1; //de-register from the Reactor.
        }
        else if(recvedSize == 0)
        {
            if(m_eventHandler != nullptr)
            {
                m_eventHandler->onLost(this);
            }
            //this->close(); //auto close when return -1 
            return -1; //de-register from the Reactor.
        }
        else
        {
            m_buffer.skipn(recvedSize);
        }
    }    

    if(m_eventHandler != nullptr)
    {
        m_eventHandler->onData(this, &m_buffer);
    }

    //keep yourself registered
    return 0;
}

int AceSocket::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
    ACE_TRACE ("AceSocket::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)");
        
    if(mask == WRITE_MASK)
        return 0;

    if (m_eventHandler != nullptr && handle != ACE_INVALID_HANDLE)
    {
        m_eventHandler->onClosed(this);
    }

    //return ParentHandler::handle_close(handle, mask); //will delete self
    return 0;
}

int AceSocket::handle_timeout(const ACE_Time_Value &time, const void *act)
{
    if (m_eventHandler != nullptr)
    {
        m_eventHandler->onTimeout(this);
    }

    return ParentHandler::handle_timeout(time, act);
}

int AceSocket::handle_exception(ACE_HANDLE fd)
{
    if (m_eventHandler != nullptr)
    {
        m_eventHandler->onFailure(this);
    }

    return ParentHandler::handle_exception(fd);
}

int AceSocket::handle_signal(int signum, siginfo_t * siginfo, ucontext_t * ctx)
{
    return ParentHandler::handle_signal(signum, siginfo, ctx);
}

int AceSocket::handle_qos(ACE_HANDLE fd)
{
    return ParentHandler::handle_qos(fd);
}

ACE_Reactor* AceSocket::getDefaultReactor()
{
    return ACE_Reactor::instance();
}

}