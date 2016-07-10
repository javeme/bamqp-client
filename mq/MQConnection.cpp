#include <iostream>
#include <assert.h>
#include "MQConnection.h"
#include "network/NetworkLoop.h"
/**
 *  Class describing a (mid-level) AMQPConnection implementation
 *
 *  @copyright 2014 lizhangmei
 */

/**
 *  Set up namespace
 */
namespace AMQP {
  
//event Loop
void eventLoop()
{
    Network::NetworkLoop::instance().init();
    Network::NetworkLoop::instance().run();
}


/*******************************************************************************************/
//MQConnection
MQConnection::MQConnection() : m_connection(nullptr), m_bOpened(false), m_bLogined(false)
{
    Network::Socket* socket = Network::SocketFactory::instance().getSocket("TCP-ACE");
    if(socket == nullptr)
        throw MQException("Socket is null [SocketFactory.getSocket()]");
    m_socket = dynamic_cast<Network::TcpSocket*>(socket);
    if(m_socket == nullptr)
    {
        Network::SocketFactory::instance().releaseSocket(socket);
        throw MQException("Socket is not TcpSocket [SocketFactory.getSocket()]");
    }
    m_socket->setEventLoop(&Network::NetworkLoop::instance());//@TODO 传入EventLoop参数
    m_socket->setEventHandler(this); 
}

MQConnection::MQConnection(Network::TcpSocket* socket) 
    : m_socket(socket), m_connection(nullptr), m_bOpened(true), m_bLogined(false)
{
    if(m_socket == nullptr)
        throw MQException("Socket is null [MQConnection.MQConnection()]");

    m_socket->setEventLoop(&Network::NetworkLoop::instance());//@TODO 传入EventLoop参数
    m_socket->setEventHandler(this); 
}

MQConnection::~MQConnection()
{
    (void)close();
    Network::SocketFactory::instance().releaseSocket(m_socket);
}

bool MQConnection::open(const std::string &host)
{
    m_hostName = host;

    int ret = m_socket->connect(host.c_str(), 5672);
    if(ret != 0)
        return false;
    //setOpened(ret == 0);//@TODO delay set

    return isOpened();
}

bool MQConnection::disOpen()
{
    int ret = m_socket->close();
    if(ret != 0)
        return false;
    //setOpened(false);//@TODO delay set
    while(isOpened()){
        printf("MQConnection::disOpen: wait close... \n");
        Network::Thread::sleep(1000);
    }

    return isOpened();
}

bool MQConnection::connect(const std::string &user, const std::string &password, const std::string &vhost)
{
    if(!isOpened())
        return false;
    if(m_connection!= nullptr) 
        return true;
    m_user = user;
    m_password = password;
    m_vhost = vhost;
    m_connection= new Connection(this, Login(user, password), vhost);
    //setLogined((m_connection!= nullptr));
    return isLogined();
}

bool MQConnection::disconnect()
{
    if(m_connection != nullptr)
    {
        m_connection->close();
        delete m_connection;
        m_connection = nullptr;
    }
    //setLogined(false);
    return !isLogined();
}

bool MQConnection::reconnect()
{
    (void)disconnect();
    (void)disOpen();

    (void)open(m_hostName);
    (void)connect(m_user, m_password, m_vhost);

    return isLogined();
}

bool MQConnection::close()
{
    /*delay set
    setLogined(!disconnect());

    if(isOpened())
        setOpened(!disOpen());*/

    (void)disconnect();
    if(isOpened())
        (void)disOpen();

    return !isOpened();
}

void MQConnection::setOpened(bool val)
{
    m_bOpened = val;
    if(!m_bOpened)
        setLogined(false);    
}

Channel* MQConnection::newChannel()
{
    if(!isLogined())
        return nullptr;
    return new Channel(m_connection);
}

//NetworkEventHandler回调
void MQConnection::onFailure(Network::Socket *socket)
{
    // show
    std::cout << "socket onFailure" << std::endl;
}

void MQConnection::onTimeout(Network::Socket *socket)
{
    // show
    std::cout << "socket onTimeout" << std::endl;
}

void MQConnection::onConnected(Network::Socket *socket)
{
    // show
    std::cout << "socket connected" << std::endl;

    setOpened(true);
}

void MQConnection::onClosed(Network::Socket *socket)
{
    // show
    std::cout << "socket closed" << std::endl;

    setOpened(false);
}

void MQConnection::onLost(Network::Socket *socket)
{
    // show
    std::cout << "socket onLost" << std::endl;

    setOpened(false);
}

void MQConnection::onData(Network::Socket *socket, Network::Buffer *buffer)
{
    std::cout << "received bytes size: " << buffer->size() << std::endl;
    
    // leap out if there is no connection
    if (!m_connection) 
        return;

    // let the data be handled by the connection
    size_t bytes = m_connection->parse(buffer->data(), buffer->size());

    // shrink the buffer
    buffer->shrink(bytes);
}

//ConnectionHandler回调
void MQConnection::onConnected(Connection *connection)
{
    // show
    std::cout << "AMQP login success" << std::endl;

    setLogined(true);
}

void MQConnection::onData(Connection *connection, const char *buffer, size_t size)
{
    // show
    std::cout << "AMQP send data, size: " << size << std::endl;

    if (!isLogined())
    {
         std::cout << "AMQP warning: not logined!" << std::endl;
    }

    // send to the socket
    m_socket->send(buffer, size);
}

void MQConnection::onError(Connection *connection, const char *message)
{
    // report error
    std::cout << "AMQP Connection error: " << message << std::endl;
}

void MQConnection::onClosed(Connection *connection)
{
    // report close
    std::cout << "AMQP Connection closeed" << std::endl;

    setLogined(false);
}


/*******************************************************************************************/
//MQConnectionPool
MQConnectionPool::MQConnectionPool()
{
    ;
}

MQConnectionPool::~MQConnectionPool()
{
    closeAllConnection();
}

MQConnectionPool* MQConnectionPool::instance()
{
    static MQConnectionPool pool;
    return &pool;
}

MQConnection* MQConnectionPool::getConnection(const std::string &host, const std::string &user, const std::string &password, const std::string &vhost)
{
    auto itor = m_mapConnection.find(host);
    if(itor != m_mapConnection.end())
    {
        return itor->second;
    }
    else
    {
        MQConnection* conn = new MQConnection();
        bool seccuss = m_mapConnection.insert(make_pair(host, conn)).second;
        assert(seccuss);
        
        if(conn != nullptr)
        {
            seccuss = conn->open(host);
            seccuss = conn->connect(user, password, vhost);
        }

        return conn;
    }
}

bool MQConnectionPool::closeConnection(MQConnection* conn)
{
    if(conn == nullptr)
        return false;

    //close
    conn->close();

    //delete
    auto itor = m_mapConnection.find(conn->getHostName());
    if(itor != m_mapConnection.end())
    {
        delete itor->second;
        m_mapConnection.erase(itor);
        return true;
    }
    else
    {
        delete conn;
        return true;
    }
    return false;
}

void MQConnectionPool::closeAllConnection()
{
    for(auto itor = m_mapConnection.begin(); itor != m_mapConnection.end(); /*++itor*/)
    {
        MQConnection* conn = itor->second;
        delete conn;
        itor = m_mapConnection.erase(itor);
    }
    m_mapConnection.clear();
}

Channel* MQConnectionPool::createChannel(const std::string &host, const std::string &user, const std::string &password, const std::string &vhost)
{
    MQConnection* conn = getConnection(host, user, password, vhost);
    if(conn == nullptr)
        return nullptr;
    return conn->newChannel();
}

void MQConnectionPool::destroyChannel(Channel* channel)
{
    //channel->close();
    delete channel;
}




/*******************************************************************************************/
//ChannelFactory
ChannelFactory::ChannelFactory(const std::string &host)
{
    m_connection= MQConnectionPool::instance()->getConnection(host);
}

ChannelFactory::ChannelFactory(const std::string &host, const std::string &user, const std::string &password, const std::string &vhost)
{
    m_connection= MQConnectionPool::instance()->getConnection(host, user, password, vhost);
}

ChannelFactory::ChannelFactory(MQConnection* conn) : m_connection(conn)
{
    ;
}

Channel* ChannelFactory::createChannel()
{
    if(m_connection == nullptr)
        return nullptr;
    return m_connection->newChannel();
}

void ChannelFactory::destroyChannel(Channel* channel)
{
    //channel->close();
    delete channel;
}

void ChannelFactory::destroyConnection()
{
    MQConnectionPool::instance()->closeConnection(m_connection);
    m_connection = nullptr;
}


}