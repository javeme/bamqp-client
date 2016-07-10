#include "PublisherConsumer.h"

namespace AMQP{

class IOCPLogHandler : public IOCPHandler
{
public:
    virtual uint16_t onNegotiate(IOCPConnection *connection, uint16_t interval) 
    {
        printf("AMQP.onNegotiate()\n");
        return interval;
    }

    virtual void onConnected(IOCPConnection *connection) 
    {
        printf("AMQP.onConnected()\n");
    }

    virtual void onHeartbeat(IOCPConnection *connection) 
    {
        printf("AMQP.onHeartbeat()\n");
    }

    virtual void onError(IOCPConnection *connection, const char *message) 
    {
        printf("AMQP.onError(%s)\n", message);
    }

    virtual void onClosed(IOCPConnection *connection) 
    {
        printf("AMQP.onClosed()\n");
    }
};

}

void testPublisher(int times=1)
{
    printf("publishing...\n");
    AMQP::Address addr("amqp://guest:guest@127.0.0.1:5672/");
    AMQP::IOCPLogHandler handler;
    AMQP::IOCPConnection conn(addr, &handler);
    conn.start();

    AMQP::Connection* connection = conn.connection();

    //Publisher (throw Exception)
    AMQP::Publisher* publisher = new AMQP::Publisher(connection, "exchange1", "key1");

    for (int i = 0; i < times; i++)
    {
        bluemei::String msg = bluemei::String::format("hello, this is the %d time!\n", i);
        bool sucess = publisher->publish(msg.c_str(), msg.length(), "test");

        printf("[%d] publish message success: %d!\n", i, sucess);
    }

    getchar();
    conn.stop();
    delete publisher;
}


void testConsumer()
{
    printf("consuming...\n");
    AMQP::Address addr("amqp://guest:guest@127.0.0.1:5672/");
    AMQP::IOCPLogHandler handler;
    AMQP::IOCPConnection conn(addr, &handler);
    conn.start();

    AMQP::Connection* connection = conn.connection();

    //Consumer (throw Exception)
    AMQP::Consumer* consumer = new AMQP::Consumer(connection, "queue1", "queue1-tag");
    consumer->bindExchange("exchange1", AMQP::topic, "key1").onSuccess([](){
        printf("bind consumer to exchange success!\n");
    });

    consumer->subscribe([&](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
        printf("\n--------------------------------------------- \n");
        bluemei::String now = bluemei::Date::getCurrentTime().toString();
        printf("consumer received message, size: %llu bytes [%s]\n",
            message.bodySize(), now.c_str());

        const char* body = message.body();
        uint64_t len = message.bodySize();
        printf("recved data:\n%s\n\n", message.message().c_str());

        std::string messageType = message.typeName();
        printf("message type: [%s]\n", messageType.c_str());

        consumer->channel()->ack(deliveryTag);
    });

    getchar();
    conn.stop();
    delete consumer;
}

int main(int argc, char* args[])
{
    bluemei::SocketTools::initSocketContext();

    bool publish = false;
    int publishTimes = 1;
    if(argc > 1)
        publish = bluemei::String(args[1]) == "publish";
    if(argc > 2 && publish)
        publishTimes = atoi(args[2]);

    try {
        if(publish)
            testPublisher(publishTimes);
        else
            testConsumer();
    } catch (bluemei::Exception& e) {
        e.printException();
    }

    bluemei::SocketTools::cleanUpSocketContext();

    system("pause");
    return 0;
}