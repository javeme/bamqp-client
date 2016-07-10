#pragma once
#include "IOCPConnection.h"
#include "ChannelFactory.h"

/**
 *  Class describing a (mid-level) AMQPConnection implementation
 *
 *  @copyright 2014 Javeme
 */

/**
 *  Set up namespace
 */
namespace AMQP {

class IOCPChannelFactory : public ChannelFactory
{
public:
    IOCPChannelFactory(const char* address, IOCPHandler* handler=nullptr);
    virtual ~IOCPChannelFactory();
public:
    virtual Connection* connection();
    virtual Channel* createChannel();
    virtual void destroyChannel(Channel* channel);
private:
    IOCPConnection m_connection;
};

}
