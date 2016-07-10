#include <stdexcept>
#include "AceThreadImpl.h"
#include "Thread.h"
#include "ace/Task.h"
#include "ace/Time_Value.h"
#include "ace/OS_NS_unistd.h"

namespace Network
{

ACE_Thread_Manager AceThreadImpl::s_threadManager;

AceThreadImpl::AceThreadImpl(Thread* parent) : m_parent(parent)
{
    //thread name
    const char** thrdName = nullptr;
    std::string strName = (m_parent != nullptr) ? m_parent->getName() : "";
    const char* tmp = strName.c_str();
    if(m_parent != nullptr){        
        thrdName = &tmp;
    }

    //create thread
    int grpId = s_threadManager.spawn(
        &AceThreadImpl::aceThreadCallback,
        (void *) this, //arg
        THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED | THR_SUSPENDED, //flag THR_SCHED_DEFAULT THR_SUSPENDED
        &m_threadId, //t_id
        0, //t_handle
        ACE_DEFAULT_THREAD_PRIORITY, //priority
        -1, //grp_id
        0, //stack
        ACE_DEFAULT_THREAD_STACKSIZE, //stack_size
        thrdName//thr_name
        );
    //failed
    if (grpId == -1)
    {
        throw std::runtime_error("Spawn a ACE thread failed");
    }
    //suspend,Some OS may ignore THR_SUSPENDED
    //(void) this->suspend();
}

AceThreadImpl::~AceThreadImpl(void)
{
    //m_threadManager.close();
}

int AceThreadImpl::sleep(unsigned long msec)
{
    //ms->(s, us)
    return ACE_OS::sleep(ACE_Time_Value(msec / 1000, (msec % 1000) * 1000));
}

void AceThreadImpl::yield()
{
    return ACE_Thread::yield();
}

int AceThreadImpl::waitAll(unsigned long msec)
{
    ACE_Time_Value t(msec / 1000, (msec % 1000) * 1000);
    ACE_Time_Value* waitMaxTime = msec == 0 ? nullptr : &t;
    return s_threadManager.wait(waitMaxTime, false, false);
}

ThreadId AceThreadImpl::currentId()
{
    return (ThreadId)ACE_Thread::self();
}

int AceThreadImpl::start()
{
    return s_threadManager.resume(m_threadId);
}


int AceThreadImpl::stop()
{
    return s_threadManager.kill(m_threadId, 100);
}

int AceThreadImpl::resume()
{
    return s_threadManager.resume(m_threadId);
}

int AceThreadImpl::suspend()
{
    return s_threadManager.suspend(m_threadId);
}

int AceThreadImpl::join()
{
    return s_threadManager.join(m_threadId);
}

int AceThreadImpl::exit()
{
    ACE_THR_FUNC_RETURN status = s_threadManager.exit();
#if defined (ACE_HAS_INTEGRAL_TYPE_THR_FUNC_RETURN)    
    return static_cast<int> (status);
#else
    return reinterpret_cast<int> (status);
#endif
}

ThreadId AceThreadImpl::getThreadId() const
{
    return ThreadId(m_threadId);
}

int AceThreadImpl::run()
{
    if(m_parent != nullptr) 
        return m_parent->run();

    return -1;
}

ACE_THR_FUNC_RETURN AceThreadImpl::aceThreadCallback(void *args)
{
    ACE_TRACE ("AceThreadImpl::aceThreadCallback");

    AceThreadImpl *t = (AceThreadImpl*) args;

    int ret = -1;
    if(t != nullptr)
    {
        // Call the run method.
        ret = t->run();
    }


    ACE_THR_FUNC_RETURN status;
#if defined (ACE_HAS_INTEGRAL_TYPE_THR_FUNC_RETURN)
    // Reinterpret case between integral types is not mentioned in the C++ spec
    status = static_cast<ACE_THR_FUNC_RETURN> (ret);
#else
    status = reinterpret_cast<ACE_THR_FUNC_RETURN> (ret);
#endif /* ACE_HAS_INTEGRAL_TYPE_THR_FUNC_RETURN */

    
    return status;
}

}
