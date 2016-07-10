/**
 *  Class describing a (mid-level) AMQP Publisher and Consumer implementation
 *
 *  @author Javeme Lee <javaloveme@gmail.com.com>
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "IOCPConnection.h"


/**
 *  Set up namespace
 */
namespace AMQP {


class MessageChannel
{
public:
    MessageChannel(Connection* conn) : m_connection(conn)
    {
        m_channel = new Channel(m_connection);
    }
    virtual ~MessageChannel()
    {
        delete m_channel;
        m_channel = nullptr;
    }
public:
    Channel* channel() const {
        bluemei::checkNullPtr(m_channel);
        return m_channel;
    }

    static int debug(const std::string& str) {
        return IOCPConnection::debug(str.c_str());
    }

private:
    Connection* m_connection;
    Channel* m_channel;
};

class Consumer : public MessageChannel
{
public:
    Consumer(Connection* conn,
        const std::string& queueName, const std::string& tag)
        : MessageChannel(conn), m_queueName(queueName), m_tag(tag),
        m_deleteExchange(false), m_deleteQueue(false)
    {  
        SuccessCallback success = [this]() { 
            // TODO: return a Deferred
            debug("queue declared: " + m_queueName); 
        };
        channel()->declareQueue(m_queueName).onSuccess(success);
    }

    virtual ~Consumer()
    {
        volatile bool queueDeleting = false, exchangeDeleting = false;
        if(m_deleteQueue) {
            queueDeleting = true;
            std::string queueName = m_queueName;
            channel()->removeQueue(m_queueName)
            .onSuccess((SuccessCallback)[=](){
                // TODO: return a Deferred
                debug("queue removed: " + queueName);
            })
            .onFinalize([&](){
                queueDeleting = false;
            });
        }

        if(m_deleteExchange) {
            exchangeDeleting = true;
            std::string exchange = m_exchange;
            channel()->removeExchange(m_exchange).onSuccess([=](){
                // TODO: return a Deferred
                debug("exchange removed: " + exchange);
            }).onFinalize([&](){
                exchangeDeleting = false;
            });
        }

        while(queueDeleting && exchangeDeleting) {
            debug("~Consumer(): waiting for deleting...");
            bluemei::Thread::sleep(100);
        }
    }
public:
    virtual Deferred& bindExchange(const std::string &exchange, 
        ExchangeType type=AMQP::direct, const std::string &bindingkey="")
    {
        if(!channel()->connected())
            // TODO: return a Deferred
            throw bluemei::Exception("Channel is ont connected [Consumer.Consumer()]");
        // declare an exchange
        channel()->declareExchange(exchange, type).onSuccess([=]() {
            // TODO: do bindQueue() and return a Deferred
            this->m_exchange = exchange;
            debug("exchange declared: " + exchange);
        });

        // bind queue and exchange
        return channel()->bindQueue(exchange, m_queueName,
            bindingkey.empty() ? m_queueName : bindingkey);
    }
    virtual DeferredConsumer& subscribe(const MessageCallback &msgCallback)
    {
        if(!channel()->connected())
            // TODO: return a Deferred
            throw bluemei::Exception("Channel is ont connected [Consumer.Consumer()]");
        // to consume message of queue
        auto& consumer = channel()->consume(m_queueName, m_tag);
        consumer.onMessage(msgCallback);
        
        //const ErrorCallback &errorCallback
        //consumer.onError(errorCallback)
        return consumer;
    }
    virtual DeferredCancel& cancel()
    {
        return channel()->cancel(m_tag);
    }
public:
    std::string getQueueName() const { return m_queueName; }
    //void setQueueName(const std::string& val) { m_queueName = val; }

    void setDeleteExchange(bool val) { m_deleteExchange = val; }
    void setDeleteQueue(bool val) { m_deleteQueue = val; }
private:
    std::string m_queueName;
    std::string m_tag;

    std::string m_exchange; //just for delete

    bool m_deleteExchange, m_deleteQueue;
};

class Publisher : public MessageChannel
{
public:
    Publisher(Connection* conn,
        const std::string &exchange, const std::string &routingKey)
        : MessageChannel(conn), m_exchange(exchange), m_routingKey(routingKey)
    {
    }
    virtual ~Publisher()
    {
    }
public:
    virtual bool publish(const char *data, size_t size,
        const char *messageType="")
    {
        if(!channel()->connected())
            return false;
        Envelope envelope(data, size);
        envelope.setTypeName(messageType);
        return this->publish(envelope);
    }

    virtual bool publish(const Envelope& envelope)
    {
        if(!channel()->connected())
            return false;
        return channel()->publish(m_exchange, m_routingKey, envelope);
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