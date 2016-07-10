#include "asn-buf.h"
#include "user.h"
using namespace SNACC;

#include "mq/PublisherConsumer.h"
#include "network/MainLoop.h"
#include "network/NetworkLoop.h"

// MQ-Asn1.cpp : 定义控制台应用程序的入口点。


class TestUser
{
public:
    TestUser(AMQP::Publisher* publisher) : m_publisher(publisher) {}
    virtual ~TestUser(){}
public:
    void login()
    {
        AsnUserLoginReqeust request;
        request.name = "nova-user";
        request.password = "123456";

        AsnBuf buf;
        AsnLen len = request.BEnc(buf);
        char* data = buf.GetSeg(len);

        m_publisher->publish("AsnUserLoginReqeust", data, (size_t)len);

        delete[] data;
    }
    
    void loginResponse(bool success)
    {
        static int counter = 0;

        AsnUserLoginResponse response;
        response.success = success;
        response.error = "no error";
        response.counter = counter++;

        AsnBuf buf;
        AsnLen len = response.BEnc(buf);
        char* data = buf.GetSeg(len);

        m_publisher->publish("AsnUserLoginResponse", data, (size_t)len);

        delete[] data;
    }

    AsnUserLoginReqeust parseLoginReqeust(const char* data, size_t len)
    {
        AsnBuf buf(data, len);

        AsnUserLoginReqeust request;

        AsnLen usedLen;
        request.BDec(buf, usedLen);

        return request;
    }

    AsnUserLoginResponse parseLoginResponse(const char* data, size_t len)
    {
        AsnBuf buf(data, len);

        AsnUserLoginResponse response;

        AsnLen usedLen;
        response.BDec(buf, usedLen);

        return response;
    }

    bool doData(const std::string& messageType, const char* data, size_t len)
    {
        if(messageType == "AsnUserLoginReqeust")
        {
            AsnUserLoginReqeust request = parseLoginReqeust(data, len);

            request.Print(std::cout);
        }
        else if(messageType == "AsnUserLoginResponse")
        {
            AsnUserLoginResponse response = parseLoginResponse(data, len);

            response.Print(std::cout);

            return response.success;
        }
        else
        {
            printf("can't deal with message type: %s \n", messageType.c_str());
        }

        return false;
    }
private:
    AMQP::Publisher* m_publisher;
};

void testUser()
{
    //ChannelFactory
    //AMQP::ChannelFactory* channelFactory = new AMQP::ChannelFactory("10.153.148.15", "guest", "imcimc");
    AMQP::ChannelFactory* channelFactory = new AMQP::ChannelFactory("10.153.88.251", "tian", "123456");
    
    //Publisher (throw Exception)
    AMQP::Publisher* publisher = new AMQP::Publisher(channelFactory, "exchange1", "key1");

    //Tester
    TestUser tester(publisher);

    //Consumer (throw Exception)
    AMQP::Consumer* consumer = new AMQP::Consumer(channelFactory, "queue1", "queue1-tag");
    consumer->bindExchange("exchange1", AMQP::topic, "key1").onSuccess([](){
        printf("bind exchange success!\n");
    });
    
    bool responsed = false;
    consumer->subscribe([&](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
        printf("\n--------------------------------------------- \n");
        printf("consumer received message, size : %ld \n", message.bodySize());

        const char* buf2 = message.body();
        int len = message.bodySize();

        const std::string& messageType = (const std::string&) message.headers()["messageType"];
        printf("message type : %s \n", messageType.c_str());
        
        responsed = tester.doData(messageType, buf2, len);

        consumer->getChannel()->ack(deliveryTag);
    });

    printf("login... \n");
    tester.login();


    printf("login response(false)... \n");
    tester.loginResponse(false);

    printf("login response(true)... \n");
    tester.loginResponse(true);

    while(!responsed)
    {
        printf("wait for response... \n");
        Network::Thread::sleep(1000);//ms
    }

    printf("test user finished \n");

    delete publisher;
    delete consumer;

    channelFactory->destroyConnection();
    delete channelFactory;
}

void testMQ()
{
    Network::Thread thrd(
        [&]() -> int
        {
            printf("testUser-Thread thread start... \n");
            try
            {
                testUser();
            }
            catch (AMQP::Exception& e)
            {
                printf("AMQP Exception: %s \n", e.what());
            }
            catch (...)
            {
                printf("Exception: %s \n", "unknown exception");
            }

            printf("testUser-Thread finished \n");
            return 0;
        }, 
        "testUser-Thread");

    thrd.start();

    printf("starting Event Loop... \n");
    AMQP::eventLoop();
}

class KeyboardEventHandle : public Network::EventHandle
{
public:
    KeyboardEventHandle(Network::EventLoop* loop) : m_loop(loop), m_count(0) 
    {
        m_loop->registerEventHandle(this);
    }
    virtual ~KeyboardEventHandle()
    {
        m_loop->unregisterEventHandle(this);
    }

    virtual int handleEvents()
    {
        m_count++;
        if (m_count % 500 == 0)
        {
            printf("do you want to exit?(y/n)\n");
            std::string str;
            std::cin >> str; 
            if(str == "y" || str == "exit"){
                Network::MainLoop::instance().stop();
                return -1;
            }
            printf("continue...\n");
        }

        return 0;
    }
private:
    Network::EventLoop* m_loop;
    int m_count;
};

int main(int argc , char* agrv[])
{
    printf("starting test mq... \n");
    
    Network::Thread testMQThread(
        [&]() -> int
        {
            printf("testMQ-Thread start... \n");

            testMQ();

            printf("testMQ-Thread finished \n");
            return 0;
        }, 
        "testMQ-Thread");

    testMQThread.start();

    KeyboardEventHandle keyboardEventHandle(&Network::MainLoop::instance());
    Network::MainLoop::instance().run();

    Network::NetworkLoop::instance().stop();
    Network::Thread::waitAll();

    printf("end of test mq. \n");
    //system("pause");
    return 0;
}

