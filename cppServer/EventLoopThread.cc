#include"EventLoopThread.h"
#include"EventLoop.h"

EventLoopThread::EventLoopThread()
    : loop_(NULL),
      thread_(std::bind(&EventLoopThread::threadFunc,this),"EventLoopThread"),
      mutex_(),
      cond_(mutex_)
{
}

EventLoopThread::~EventLoopThread()
{
        loop_->quit();
        thread_.join();
}

EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        while(loop_ == NULL)            // until threadFunc execute
            cond_.wait();
    }
    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;     // a stack object
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    loop.loop();      
    loop_ = NULL;
}
