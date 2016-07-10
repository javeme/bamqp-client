#pragma once
#include "MQConnection.h"
/**
 *  Class describing a (mid-level) AMQP Publisher and Consumer implementation
 *
 *  @copyright 2014 lizhangmei
 */

/**
 *  Set up namespace
 */
namespace AMQP {


class MessageChannel
{
public:
    MessageChannel(AMQP::ChannelFactory* channelFactory)
        : m_channelFactory(channelFactory)
    {
        m_channel =  m_channelFactory->createChannel();
        for (int i = 0; i < 10 && m_channel == nullptr; i++)
        {
            printf("wait for connection (%d) ...\n", i);
            Network::Thread::sleep(1000);//ms
            m_channel = m_channelFactory->createChannel();
        }

        if(m_channel == nullptr)
            throw AMQP::MQException("Channel is null [MessageChannel.MessageChannel()]");
    }
    virtual ~MessageChannel()
    {
        m_channelFactory->destroyChannel(m_channel);
        m_channel = nullptr;
    }
public:
    AMQP::Channel* getChannel() const { return m_channel; }
    void setChannel(AMQP::Channel* val) { if(val == nullptr)return; m_channel = val; }
protected:
    AMQP::ChannelFactory* m_channelFactory;
    AMQP::Channel* m_channel;
};

class Consumer : public MessageChannel
{
public:
    Consumer(AMQP::ChannelFactory* channelFactory, const std::string& queueName, const std::string& tag)
        : MessageChannel(channelFactory), m_queueName(queueName), m_tag(tag)
    {  
        m_channel->declareQueue(m_queueName).onSuccess([this]() { 
            std::cout << "queue declared: " << m_queueName << std::endl; 
        });
    }
    virtual ~Consumer()
    {
    }
public:
    virtual AMQP::Deferred& bindExchange(const std::string &exchange, AMQP::ExchangeType type = AMQP::direct, 
        const std::string &routingkey = "")
    {
        if(!m_channel->connected())
            throw AMQP::MQException("Channel is ont connected [Consumer.Consumer()]");
        // declare an exchange
        m_channel->declareExchange(exchange, type).onSuccess([=]() { 
            std::cout << "exchange declared: " << exchange << std::endl;
        });

        // bind queue and exchange
        return m_channel->bindQueue(exchange, m_queueName, routingkey.empty() ? m_queueName : routingkey);
    }
    virtual AMQP::DeferredConsumer& subscribe(const AMQP::MessageCallback &msgCallback)
    {
        if(!m_channel->connected())
            throw AMQP::MQException("Channel is ont connected [Consumer.Consumer()]");
        // to consume message of queue
        auto& consumer = m_channel->consume(m_queueName, m_tag);
        consumer.onMessage(msgCallback);
        
        //const AMQP::ErrorCallback &errorCallback
        //consumer.onError(errorCallback)
        return consumer;
    }
    virtual AMQP::DeferredCancel& cancel()
    {
        return m_channel->cancel(m_tag);
    }
public:
    std::string getQueueName() const { return m_queueName; }
    //void setQueueName(const std::string& val) { m_queueName = val; }
private:
    std::string m_queueName;
    std::string m_tag;
};

class Publisher : public MessageChannel
{
public:
    Publisher(AMQP::ChannelFactory* channelFactory, const std::string &exchange, const std::string &routingKey)
        : MessageChannel(channelFactory), m_exchange(exchange), m_routingKey(routingKey)
    {        
    }
    virtual ~Publisher()
    {
    }
public:
    virtual bool publish(const char *messageType, const char *data, size_t size)
    {
        if(m_channel == nullptr || !m_channel->connected())
            return false;
        AMQP::Envelope envelope(data, size);
        //envelope.setTypeName(messageType);
        AMQP::Table headers;
        headers["messageType"] = messageType;
        envelope.setHeaders(headers);
        return m_channel->publish(m_exchange, m_routingKey, envelope);
    }

public:
    std::string getExchange() const { return m_exchange; }
    //void setExchange(const std::string& val) { m_exchange = val; }

    std::string getRoutingKey() const { return m_routingKey; }
    //void setRoutingKey(const std::string& val) { m_routingKey = val; }
private:
    std::string m_exchange;
    std::string m_routingKey;
};

}
