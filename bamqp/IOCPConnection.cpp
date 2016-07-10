/**
 *  TcpConnected.h
 * 
 *  The actual tcp connection - this is the "_impl" of a tcp-connection after
 *  the hostname was resolved into an IP address
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
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

IOCPConnection::IOCPConnection(const Address &address, IOCPHandler* handler) :
    m_socket(-1),
    m_address(address),
    m_handler(handler),
    m_outBuffer(OutputBuffer()),
    m_inBuffer(4096),
    m_connection(nullptr),
    m_alive(false),
    m_closed(false)
{
    m_iocp.setEventHandle(this);
}

IOCPConnection::~IOCPConnection()
{
    try {
        this->stop();
    } catch (bluemei::Exception& e) {
        debug("Stop Error: " + e.toString());
        this->reportError("Stop Error: " + e.toString());
    }
    this->m_iocp.setEventHandle(nullptr);
}

void IOCPConnection::start()
{
    m_socket = m_iocp.connect(m_address.hostname().c_str(), m_address.port());
    m_connection = new Connection(this, m_address.login(), m_address.vhost());
}

void IOCPConnection::stop()
{
    if(m_socket == -1)
        return;
    //close mq
    this->close();
    //NOTE: please don't stop/disconnect IOCPModel in work(task) thread!
    //close socket
    m_iocp.disconnect();
    m_socket = -1;
}

void IOCPConnection::close()
{
    if(m_connection == nullptr || m_closed)
        return;
    //close mq connection
    m_connection->close();
    //wait for finishing
    bluemei::Waiter waiter([&](){ return !m_connection->waiting(); });
    waiter.wait();
    // destroy m_connection(would close mq) before iocp stoped
    delete m_connection;
    m_connection = nullptr;
}

bool IOCPConnection::onWriteFinish(bluemei::IOEvent& ev)
{
    //debug("sent %u bytes", ev.data.length());
    //TODO: remove this big lock!
    bluemei::ScopedLock sl(m_lock);

    // because the object might soon be destructed, we create a monitor to check this
    //Monitor monitor(this);

    //send next frame
    if (m_outBuffer.size() > 0)
    {
        auto buf = m_outBuffer.first();
        this->m_iocp.send(buf.data, buf.size, m_socket);
        m_outBuffer.shrink(buf.size);
    }

    //needs to next sent if out buffer is not empty
    return m_outBuffer.size() > 0;
}

bool IOCPConnection::onReadFinish(bluemei::IOEvent& ev)
{
    //debug("received %d bytes", ev.data.length());
	//TODO: remove this big lock! need to resolve shared vector/buffer...
	//like ConnectionImpl._channels, ChannelImpl._consumers, Watchable._monitors
	//and Publisher.publish() / Consumer.consume() may be called by other thread.
    bluemei::ScopedLock sl(m_lock);

    // because the object might soon be destructed, we create a monitor to check this
    Monitor monitor(this);

    //TODO: to support multi thread call
	//1.add lock for buffer methods.
	//2.add a method: bool inUse() by thread, another thread returns directly.
	//  or copy every package to parse.
    m_inBuffer.received(ev.data.bytes(), ev.data.length());

    while(m_inBuffer.size() >= connection()->expected()
        && !m_closed && monitor.valid())
    {
        // we need a local copy of the buffer - because it is possible that "this"
        // object gets destructed halfway through the call to the parse() method
        //InputBuffer buffer(std::move(m_inBuffer));
        InputBuffer& buffer = m_inBuffer;

        // parse the buffer
        uint64_t processed = this->parse(buffer);
        //debug("processed %d bytes", processed);
        
        // shrink buffer
        buffer.shrink(processed);

        // restore the buffer as member
        //m_inBuffer = std::move(buffer);

        //processed may be 0 after error
        if(processed == 0)
            break;
    }
    
    return true;
}


bool IOCPConnection::onWrite(bluemei::IOEvent& ev)
{
    // TODO: handle these callbacks! 
    debug("Socket.onWrite()");

    return true;
}

bool IOCPConnection::onRead(bluemei::IOEvent& ev )
{
    debug("Socket.onRead()");

    return true;
}

bool IOCPConnection::onAccepted(bluemei::IOEvent& ev)
{
    return false;
}

bool IOCPConnection::onError(bluemei::IOEvent& ev)
{
    debug("Socket.onError()");

    return this->reportError("Socket Error");
}

bool IOCPConnection::onClosed(bluemei::IOEvent& ev)
{
    this->m_alive = false;
    this->m_closed = true;

    debug("Socket.onClosed()");

    return false;
}

bool IOCPConnection::onException(bluemei::Exception& e)
{
    // TODO: handle Exception 
    //e.printException();

    return this->reportError("Socket Error: " + e.toString());
}

void IOCPConnection::send(const char *buffer, size_t size)
{
    // is there already a buffer of data that can not be sent?
    if (m_outBuffer.size() > 0) {
        m_outBuffer.add(buffer, size);
    }

    // there is no buffer, send the data right away
    this->m_iocp.send((const unsigned char*)buffer, size, m_socket);
}


///////////////////////////////////////////////////////////////////////////////
//MQ Connection callbacks

/**
 *  Method that is called when the heartbeat frequency is negotiated.
 *  @param  connection      The connection that suggested a heartbeat interval
 *  @param  interval        The suggested interval from the server
 *  @return uint16_t        The interval to use
 */
uint16_t IOCPConnection::onNegotiate(Connection *connection, uint16_t interval)
{
    if(m_handler != nullptr)
        return m_handler->onNegotiate(this, interval);
    else
        return interval;
}

/**
 *  Method that is called by the connection when data needs to be sent over the network
 *  @param  connection      The connection that created this output
 *  @param  buffer          Data to send
 *  @param  size            Size of the buffer
 */
void IOCPConnection::onData(Connection *connection, const char *buffer, size_t size)
{
    // send the data over the connection
    this->send(buffer, size);
}

/**
 *  Method that is called when the server sends a heartbeat to the client
 *  @param  connection      The connection over which the heartbeat was received
 */
void IOCPConnection::onHeartbeat(Connection *connection)
{
    // let the state object do this
    if(m_handler != nullptr)
        m_handler->onHeartbeat(this);
}

/**
 *  Method called when the connection ends up in an error state
 *  @param  connection      The connection that entered the error state
 *  @param  message         Error message
 */
void IOCPConnection::onError(Connection *connection, const char *message)
{
    // tell the implementation to report the error
    if(m_handler != nullptr)
        m_handler->onError(this, message);
}

/**
 *  Method that is called when the connection is established
 *  @param  connection      The connection that can now be used
 */
void IOCPConnection::onConnected(Connection *connection)
{
    // tell the implementation to report the status
    if(m_handler != nullptr)
        m_handler->onConnected(this);
    m_alive = true;
}

/**
 *  Method that is called when the connection was closed.
 *  @param  connection      The connection that was closed and that is now unusable
 */
void IOCPConnection::onClosed(Connection *connection)
{
    this->m_alive = false;
    this->m_closed = true;
    //this->stop();
    if(m_handler != nullptr)
        m_handler->onClosed(this);
}

bool IOCPConnection::reportError(const char *message)
{
    // some errors are ok and do not (necessarily) mean that we're disconnected
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        return false;

    // we have an error - report this to the user
    if(m_handler != nullptr) {
        message = message!=nullptr ? message : strerror(errno);
        m_handler->onError(this, message);
    }

    // done
    return true;
}

Connection* IOCPConnection::connection() const
{
    bluemei::checkNullPtr(m_connection);
    return m_connection;
}

int IOCPConnection::debug(const char* frmt, ...)
{
    if(!s_needDebug)
        return -1;

    va_list arg_ptr;
    va_start(arg_ptr, frmt);

    return vprintf(frmt, arg_ptr) + printf("\n");
}

bool IOCPConnection::s_needDebug = false;
void IOCPConnection::setDebug(bool debug)
{
    s_needDebug = debug;
}

/**
 *  End of namespace
 */
}
