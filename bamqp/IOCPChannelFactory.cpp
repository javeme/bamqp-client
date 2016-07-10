#pragma once
#include "IOCPChannelFactory.h"

/**
 *  Class describing a (mid-level) AMQPConnection implementation
 *
 *  @copyright 2014 Javeme
 */

/**
 *  Set up namespace
 */
namespace AMQP {

IOCPChannelFactory::IOCPChannelFactory(const char* address, IOCPHandler* handler)
    : m_connection(AMQP::Address(address), handler)
{
    m_connection.start();
}

IOCPChannelFactory::~IOCPChannelFactory()
{
    m_connection.stop();
}

Connection* IOCPChannelFactory::connection()
{
    return m_connection.connection();
}

Channel* IOCPChannelFactory::createChannel()
{
    return new Channel(this->connection());
}

void IOCPChannelFactory::destroyChannel(Channel* channel)
{
    //TODO: store channels for the next time
    //channel->close();
    delete channel;
}

}