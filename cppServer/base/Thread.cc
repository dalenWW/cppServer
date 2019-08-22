#include"Thread.h"
#include"CurrentThread.h"
#include<unistd.h>
#include<stdio.h>
#include<sys/syscall.h>
#include<assert.h>
#include<sys/prctl.h>
#include<cstring>

namespace CurrentThread 
{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread const char* t_threadName = "default";
}

void CurrentThread::cacheTid()
{
    if(t_cachedTid == 0)
    {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        snprintf(t_tidString,sizeof t_tidString,"%5d ",t_cachedTid);
    }
}

struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;

    ThreadData(const ThreadFunc& func,std::string& name,pid_t* tid)
    : func_(func),
      name_(name),
      tid_(tid)
      {}
    
    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        CurrentThread::t_threadName = name_.empty() ? "Thread": name_.c_str();
        ::prctl(PR_SET_NAME,CurrentThread::t_threadName);
        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void* startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

void Thread::setDefaultName()
{
    if(name_.empty())
    {
        char buf[32];
        snprintf(buf,sizeof buf,"Thread");
        name_ = buf;
    }
}

Thread::Thread(const ThreadFunc& func,const std::string& name)
    :started_(false),
     tid_(0),
     pthreadId_(0),
     func_(func),
     name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    ThreadData *data = new ThreadData(func_,name_,&tid_);
    if(pthread_create(&pthreadId_,NULL,&startThread,data))
    {
        started_ = false;
        delete data;
    } 
    // else
    // {
    //     assert(tid_ > 0);
    // }
}

int Thread::join()
{
    assert(started_);
    return pthread_join(pthreadId_,NULL);
}
