// Network.cpp : 定义控制台应用程序的入口点。
//


#include "Network.h"
#include "AceSocket.h"
#include "MainLoop.h"
#include "Thread.h"
using namespace Network;

class MQConnection : public NetworkEventHandler//, public AMQP::ConnectionHandler
{
public:
    virtual void onFailure( Socket *socket )
    {
        printf("socket onFailure \n");
    }

    virtual void onTimeout( Socket *socket )
    {
        printf("socket onTimeout \n");
    }

    virtual void onConnected( Socket *socket )
    {
        printf("socket connected \n");
    }

    virtual void onClosed( Socket *socket )
    {
        printf("socket closed \n");
    }

    virtual void onLost( Socket *socket )
    {
        printf("socket onLost \n");
    }

    virtual void onData( Socket *socket, Buffer *buffer )
    {
        printf("received %d bytes \n", buffer->size());
        printf("<=======\n%s\n>=======\n\n", buffer->data());
        buffer->shrink(buffer->size());
    }

};

int testSend()
{
    NetworkEventHandler* eventHandle = new MQConnection();
    AceSocket tcpSocket(&MainLoop::instance(), eventHandle);
    AceSocket::getDefaultReactor()->owner((ACE_thread_t)MainLoop::instance().getThreadId());

    int ret = tcpSocket.connect("10.153.88.251", 55672);//10.153.88.251 80 5672
    
    if(ret == -1)  
    {  
        int iErr = ACE_OS::set_errno_to_wsa_last_error();
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("connect error!\n")));
        //ACE_ERROR(LM_ERROR, "%P|%t, %p", "Connection failed!\n");

        system("pause");
        return -1;
    }

    const char* request =  "GET / HTTP/1.1\n"
        "Accept-Language: zh-cn\n"
        "Accept-Encoding: gzip, deflate\n"
        "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727)\n"
        "Connection: Keep-Alive\n"
        "\n";

    int count = 0;
    
    for(int i=0; i<100; i++)
    {
        int size = tcpSocket.send(request, strlen(request));
        printf("sended %d bytes \n", size);

        count += size;

        Thread::sleep(1000*3);
    }
    return count;
}

int main(int argc , char* agrv[])
{
    Thread thrd(
        [&]() -> int
        {
            int sendBytes = testSend();
            return 0;
        }, 
        "testThread1");

    thrd.start();
    //thrd.suspend();
    
    MainLoop::instance().run();

    Thread::waitAll();
    //tcpSocket.close();

    return 0;
}

