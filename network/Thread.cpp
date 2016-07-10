#include "Thread.h"
#include "AceThreadImpl.h"

namespace Network
{

class ThreadImpl : public AceThreadImpl{ public: ThreadImpl(Thread* parent) : AceThreadImpl(parent) {} };

Thread::Thread(const std::string& name) : m_runer(nullptr), m_runerFunction(nullptr), m_name(name)
{
    m_impl = new ThreadImpl(this);
}

Thread::Thread(Runnable* runer, const std::string& name) : m_runer(runer), m_runerFunction(nullptr), m_name(name)
{
    m_impl = new ThreadImpl(this);
}

Thread::Thread(const RunFunction& runer, const std::string& name) : m_runer(nullptr), m_runerFunction(runer), m_name(name)
{
    m_impl = new ThreadImpl(this);
}

Thread::~Thread( void )
{
    delete m_impl;
}

int Thread::sleep(unsigned long msec)
{
    return ThreadImpl::sleep(msec);
}

void Thread::yield()
{
    return ThreadImpl::yield();
}

int Thread::waitAll(unsigned long msec)
{
    return ThreadImpl::waitAll(msec);
}

ThreadId Thread::currentId()
{
    return ThreadImpl::currentId();
}

int Thread::start()
{
    return m_impl->start();
}

int Thread::stop()
{
     return m_impl->stop();
}

int Thread::resume()
{
     return m_impl->resume();
}

int Thread::suspend()
{
     return m_impl->suspend();
}

int Thread::join()
{
     return m_impl->join();
}

int Thread::exit()
{
     return m_impl->exit();
}

ThreadId Thread::getThreadId() const
{
    return m_impl->getThreadId();
}

int Thread::run()
{
    if(m_runer != nullptr)
    {
        return m_runer->run();
    }
    else if (m_runerFunction != nullptr)
    {
        return m_runerFunction();
    }

    return 0;
}

}