#include "NetworkLoop.h"
#include "AceSocket.h"

namespace Network
{

NetworkLoop::NetworkLoop(void)
{
}

NetworkLoop::~NetworkLoop(void)
{
}

NetworkLoop& NetworkLoop::instance()
{
    static NetworkLoop loop;
    return loop;
}

void NetworkLoop::init()
{
    AceSocket::getDefaultReactor()->owner(getThreadId());
}

}