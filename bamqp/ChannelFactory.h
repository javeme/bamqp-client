#pragma once
#include <memory>

/**
 *  Class describing a (mid-level) AMQPConnection implementation
 *
 *  @copyright 2014 Javeme
 */

/**
 *  Set up namespace
 */
namespace AMQP {

class Connection;
class Channel;

/**
 *  class ChannelFactory
 */
class ChannelFactory
{
public:
    ChannelFactory() {}
    virtual ~ChannelFactory() {}
public:
    virtual Connection* connection() = 0;
    virtual Channel* createChannel() = 0;
    virtual void destroyChannel(Channel* channel) = 0;
};

}
