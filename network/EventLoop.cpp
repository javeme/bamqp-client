#include "EventLoop.h"

namespace Network
{
    
EventLoop::EventLoop() : m_bRunning(false)
{
    m_threadId = Thread::currentId();
}

EventLoop::~EventLoop()
{
    ;
}

void EventLoop::init()
{
    m_bRunning = false;
}

void EventLoop::run()
{
    if(!isInSelfThread())
    {
        return;
    }

    m_bRunning = true;
    while(isRunning())
    {
        int events = 0;
        auto listEventHandle = m_listEventHandle;
        for(auto itor = listEventHandle.begin(); itor != listEventHandle.end(); ++itor)
        {
            EventHandle* handle = *itor;
            if(handle)
            {
                int count = handle->handleEvents();
                if(count < 0)
                {
                    (void)unregisterEventHandle(handle); //remove
                }
                else
                {
                    events += count;
                }
            }
        }
        if(events == 0 || m_listEventHandle.empty())
        {
            Thread::sleep(10);
        }
    }
}

bool EventLoop::registerEventHandle(EventHandle* handle)
{
    auto itor = std::find(m_listEventHandle.begin(), m_listEventHandle.end(), handle);
    if(itor ==  m_listEventHandle.end())
    {
        m_listEventHandle.push_back(handle);
        return true;
    }
    return false;
}

bool EventLoop::unregisterEventHandle(EventHandle* handle)
{
    auto itor = std::find(m_listEventHandle.begin(), m_listEventHandle.end(), handle);
    if(itor !=  m_listEventHandle.end())
    {
        m_listEventHandle.erase(itor);
        return true;
    }
    return false;
}

void EventLoop::stop()
{
    m_bRunning = false;
}

bool EventLoop::isInSelfThread() const
{
    return (m_threadId == Thread::currentId());
}

ThreadId EventLoop::getThreadId() const
{
    return m_threadId;
}

}