#pragma once

namespace Network
{

class Socket;
class Buffer;

class NetworkEventHandler
{
public:
    /**
     *  Method that is called when the connection failed
     *  @param  socket      Pointer to the socket
     */
    virtual void onFailure(Socket *socket) =0;

    /**
     *  Method that is called when the connection timed out (which also is a failure
     *  @param  socket      Pointer to the socket
     */
    virtual void onTimeout(Socket *socket) =0;
    
    /**
     *  Method that is called when the connection succeeded
     *  @param  socket      Pointer to the socket
     */
    virtual void onConnected(Socket *socket) =0;
    
    /**
     *  Method that is called when the socket is closed (as a result of a TcpSocket::close() call)
     *  @param  socket      Pointer to the socket
     */
    virtual void onClosed(Socket *socket) =0;

    /**
     *  Method that is called when the peer closed the connection
     *  @param  socket      Pointer to the socket
     */
    virtual void onLost(Socket *socket) =0;
    
    /**
     *  Method that is called when data is received on the socket
     *  @param  socket      Pointer to the socket
     *  @param  buffer      Pointer to the fill input buffer
     */
    virtual void onData(Socket *socket, Buffer *buffer) =0;
};

}