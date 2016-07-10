#pragma once
#include <map>
#include <exception>
#include "network/Network.h"
#include "amqpcpp/src/includes.h"

/**
 *  Class describing a (mid-level) AMQPConnection implementation
 *
 *  @copyright 2014 lizhangmei
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  class MQConnection
 *  封装了socket及amqp connection的连接类
 *
 *  @copyright 2014 lizhangmei
 */
class MQConnection : 
    public ConnectionHandler,
    public Network::NetworkEventHandler
{
public:
    MQConnection();
    MQConnection(Network::TcpSocket* socket);
    virtual ~MQConnection();
public:
    //打开底层socket连接
    virtual bool open(const std::string &host);
    //关闭底层socket连接
    virtual bool disOpen();
    
    //登录MQ-connection
    virtual bool connect(const std::string &user, const std::string &password, const std::string &vhost = "/");
    virtual bool disconnect();

    //重新连接(包括socket及MQ-connection)
    virtual bool reconnect();

    //关闭MQ-connection以及底层socket
    virtual bool close();
public:
    bool isOpened() const { return m_bOpened; }
    void setOpened(bool val);

    bool isLogined() const { return m_bLogined; }
    void setLogined(bool val) { m_bLogined = val; }
public:
    Channel* newChannel();

    std::string getHostName() const { return m_hostName; }
    void setHostName(const std::string& val) { m_hostName = val; }

protected:
    //NetworkEventHandler回调
    virtual void onFailure(Network::Socket *socket);
    virtual void onTimeout(Network::Socket *socket);
    virtual void onConnected(Network::Socket *socket);
    virtual void onClosed(Network::Socket *socket);
    virtual void onLost(Network::Socket *socket);
    virtual void onData(Network::Socket *socket, Network::Buffer *buffer);

protected:
    //ConnectionHandler回调
    virtual void onConnected(Connection *connection);
    virtual void onClosed(Connection *connection);
    virtual void onData(Connection *connection, const char *buffer, size_t size);
    virtual void onError(Connection *connection, const char *message);

private:
    std::string m_hostName;
    std::string m_user;
    std::string m_password;
    std::string m_vhost;

    Network::TcpSocket* m_socket;
    Connection* m_connection;

    volatile bool m_bOpened;
    volatile bool m_bLogined;
};

/**
 *  class MQConnectionPool
 *  连接池
 *
 *  @copyright 2014 lizhangmei
 */
class MQConnectionPool
{
public:
    static MQConnectionPool* instance();

    MQConnectionPool();
    virtual ~MQConnectionPool();
public:
    MQConnection* getConnection(const std::string &host, 
        const std::string &user = "", const std::string &password = "", const std::string &vhost = "/");
    bool closeConnection(MQConnection* conn);
    void closeAllConnection();

    Channel* createChannel(const std::string &host, 
        const std::string &user, const std::string &password, const std::string &vhost);
    void destroyChannel(Channel* channel);
protected:
    std::map<std::string, MQConnection*> m_mapConnection;
};


/**
 *  class ChannelFactory
 *  Channel工厂
 *
 *  @copyright 2014 lizhangmei
 */
class ChannelFactory
{
public:
    ChannelFactory(const std::string &host);
    ChannelFactory(const std::string &host, 
        const std::string &user, const std::string &password, const std::string &vhost = "/");
    ChannelFactory(MQConnection* conn);

    virtual ~ChannelFactory() {}
public:
    Channel* createChannel();
    void destroyChannel(Channel* channel);

    void destroyConnection();
private:
    MQConnection* m_connection;
};


/**
 *  class MQException
 *  异常
 *
 *  @copyright 2014 lizhangmei
 */
class MQException : public Exception
{
public:
    MQException(const char* str) : Exception(str){}
    virtual ~MQException(){}
};

//event Loop
void eventLoop();

}
