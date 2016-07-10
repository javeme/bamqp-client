#pragma once
#include <vector>
#include <algorithm>
#include "Thread.h"

namespace Network
{

class EventHandle
{
public:
    virtual ~EventHandle(){}
    virtual int handleEvents() =0;
};

class EventLoop
{
protected:
    EventLoop();
    virtual ~EventLoop();
public:
    virtual void init();
    virtual void run();

    bool registerEventHandle(EventHandle* handle);

    bool unregisterEventHandle(EventHandle* handle);

    inline bool isRunning() const{ return m_bRunning; }
    void stop();

    bool isInSelfThread() const;
    ThreadId getThreadId() const;
private:
    ThreadId m_threadId;
    std::vector<EventHandle*> m_listEventHandle;
    volatile bool m_bRunning;
};

}