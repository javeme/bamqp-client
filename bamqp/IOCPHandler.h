/**
 *  IOCPHandler.h
 *
 *  Interface to be implemented by the caller of the AMQP library in case
 *  the "IOCP" class is being used as alternative for the ConnectionHandler
 *  class.
 * 
 *  @author Javeme Lee <javaloveme@gmail.com.com>
 */

/**
 *  Include guard
 */
#pragma once
#include <stdint.h>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class IOCPConnection;


/**
 *  Class definition
 */
class IOCPHandler
{
public:
    /**
     *  Method that is called when the heartbeat frequency is negotiated
     *  between the server and the client. 
     *  @param  connection      The connection that suggested a heartbeat interval
     *  @param  interval        The suggested interval from the server
     *  @return uint16_t        The interval to use
     *
     *  @see ConnectionHandler::onNegotiate
     */
    virtual uint16_t onNegotiate(IOCPConnection *connection, uint16_t interval)
    {
        // default implementation, suggested heartbeat is ok
        return interval;
    }

    /**
     *  Method that is called when the TCP connection ends up in a connected state
     *  @param  connection  The TCP connection
     */
    virtual void onConnected(IOCPConnection *connection) {}

    /**
     *  Method that is called when the server sends a heartbeat to the client
     *  @param  connection      The connection over which the heartbeat was received
     *
     *  @see    ConnectionHandler::onHeartbeat
     */
    virtual void onHeartbeat(IOCPConnection *connection) {}
    
    /**
     *  Method that is called when the TCP connection ends up in an error state
     *  @param  connection  The TCP connection
     *  @param  message     Error message
     */
    virtual void onError(IOCPConnection *connection, const char *message) {}
    
    /**
     *  Method that is called when the TCP connection is closed
     *  @param  connection  The TCP connection
     */
    virtual void onClosed(IOCPConnection *connection) {}
    
};
    
/**
 *  End of namespace
 */
}

