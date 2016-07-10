/**
 *  IOCPConnection.h
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

/**
 *  Dependencies
 */
#include "IOCPHandler.h"
#include "IOCPBuffer.h"

#include "blib.h"

/**
 *  Set up namespace
 */
namespace AMQP{


/**
 *  Class definition
 */
class IOCPConnection :
    protected ConnectionHandler, private Watchable,
    public bluemei::IOCPEventHandle
{
public:
    /**
     *  Constructor
     *  @param  address     MQ Address
     *  @param  handler     User-supplied handler object
     */
    IOCPConnection(const Address &address, IOCPHandler* handler=nullptr);
    
    /**
     *  Destructor
     */
    virtual ~IOCPConnection();

private:
    IOCPConnection(const IOCPConnection& other);

public:
    void start();
    void stop();
    void close();
    
    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size);
public:
    virtual bool onWrite(bluemei::IOEvent& ev);
    virtual bool onRead(bluemei::IOEvent& ev);
    virtual bool onAccepted(bluemei::IOEvent& ev);
    virtual bool onError(bluemei::IOEvent& ev);
    virtual bool onClosed(bluemei::IOEvent& ev);
    virtual bool onException(bluemei::Exception& e);

    virtual bool onWriteFinish(bluemei::IOEvent& ev);
    virtual bool onReadFinish(bluemei::IOEvent& ev);
    
protected:    
    /**
     *  Helper method to report an error
     *  @return bool        Was an error reported?
     */
    bool reportError(const char *message=nullptr);
    
    /**
     *  Parse a buffer that was received
     *  @param  buffer
     */
    uint64_t parse(const Buffer &buffer)
    {
        // pass to the AMQP connection
        bluemei::checkNullPtr(m_connection);
        return m_connection->parse(buffer);
    }

protected:
    /**
     *  Method that is called when the heartbeat frequency is negotiated.
     *  @param  connection      The connection that suggested a heartbeat interval
     *  @param  interval        The suggested interval from the server
     *  @return uint16_t        The interval to use
     */
    virtual uint16_t onNegotiate(Connection *connection, uint16_t interval) override;

    /**
     *  Method that is called by the connection when data needs to be sent over the network
     *  @param  connection      The connection that created this output
     *  @param  buffer          Data to send
     *  @param  size            Size of the buffer
     */
    virtual void onData(Connection *connection, const char *buffer, size_t size) override;

    /**
     *  Method that is called when the server sends a heartbeat to the client
     *  @param  connection      The connection over which the heartbeat was received
     */
    virtual void onHeartbeat(Connection *connection) override;

    /**
     *  Method called when the connection ends up in an error state
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onError(Connection *connection, const char *message) override;

    /**
     *  Method that is called when the connection is established
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(Connection *connection) override;

    /**
     *  Method that is called when the connection was closed.
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(Connection *connection) override;

public:
    virtual Connection* connection() const;

    static int debug(const char* frmt, ...);
    static void setDebug(bool debug);
private:
    /**
     *  The IOCP Manager
     *  @var IOCPModel
     */
    bluemei::IOCPModel m_iocp;
    bluemei::socket_t m_socket;
    bluemei::CriticalLock m_lock;

    Address m_address;
    IOCPHandler* m_handler;
    
    /**
     *  The AMQP  Connection
     *  @var Connection
     */
    Connection* m_connection;
    bool m_alive, m_closed;
    static bool s_needDebug;

    /**
     *  The outgoing buffer
     *  @var OutputBuffer
     */
    OutputBuffer m_outBuffer;
    
    /**
     *  An incoming buffer
     *  @var InputBuffer
     */
    InputBuffer m_inBuffer;
};

/**
 *  End of namespace
 */
}

