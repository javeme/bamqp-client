#pragma once
#include "EventLoop.h"

namespace Network
{

class NetworkLoop : public EventLoop
{
protected:
    NetworkLoop(void);
    virtual ~NetworkLoop(void);
public:
    static NetworkLoop& instance();

public:
    virtual void init();

};

}