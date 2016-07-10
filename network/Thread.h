#pragma once
#include <string>
#include <functional>

namespace Network
{

class ThreadImpl;

typedef unsigned long ThreadId;

#if _HAS_TR1_IMPORTS
typedef std::tr1::function<int(void)> RunFunction;
#else
typedef std::function<int(void)> RunFunction;
#endif

class Runnable
{
public:
    virtual int run() = 0;
public:
    Runnable(){}
    virtual ~Runnable(){}
};

class Thread : public Runnable
{
public:
    Thread(const std::string& name = "");
    Thread(Runnable* runer, const std::string& name = "");
    Thread(const RunFunction& runer, const std::string& name = "");
    virtual ~Thread(void);
public:
    static int sleep(unsigned long msec);
    static void yield();
    static int waitAll(unsigned long  msec = 0);

    static ThreadId currentId();
public:
    virtual int start();
    virtual int stop();

    virtual int resume();
    virtual int suspend();

    virtual int join();
    
    //exit from self thread
    virtual int exit();
public:
    std::string getName() const { return m_name; }
    void setName(const std::string& val) { m_name = val; }
    
    ThreadId getThreadId() const;

public:
    virtual int run();

protected:
    ThreadImpl* m_impl;

    std::string m_name;
    Runnable* m_runer;
    RunFunction m_runerFunction;
};

}