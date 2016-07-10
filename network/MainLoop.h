#pragma once
#include "EventLoop.h"

namespace Network
{
    
class MainLoop : public EventLoop
{
protected:
    MainLoop(){}
    virtual ~MainLoop(){}
public:
    static MainLoop& instance();
private:
    static MainLoop s_mainLoop;
};

}