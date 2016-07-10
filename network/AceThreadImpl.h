#pragma once
#include "ace/Thread_Manager.h"

namespace Network
{

class Thread;
typedef unsigned long ThreadId;

class AceThreadImpl
{
public:
    AceThreadImpl(Thread* parent);
    virtual ~AceThreadImpl(void);
public:
    static int sleep(unsigned long msec);
    static void yield();
    static int waitAll(unsigned long msec);

    static ThreadId currentId();
public:
    virtual int start();
    virtual int stop();

    virtual int resume();
    virtual int suspend();

    virtual int join();
    
    //exit from self thread
    virtual int exit();

    virtual ThreadId getThreadId() const;
protected:
    virtual int run();
public:
    static ACE_THR_FUNC_RETURN aceThreadCallback(void *args);

protected:
    Thread* m_parent;

    ACE_thread_t m_threadId;

    static ACE_Thread_Manager s_threadManager;
};

}
