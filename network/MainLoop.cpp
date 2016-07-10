#include "MainLoop.h"

namespace Network
{

Network::MainLoop MainLoop::s_mainLoop;


MainLoop& MainLoop::instance()
{
    return s_mainLoop;
}

}

