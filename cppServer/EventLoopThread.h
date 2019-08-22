#pragma once
#include"base/Condition.h"
#include"base/MutexLock.h"
#include"base/Thread.h"
#include"base/noncopyable.h"

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop* loop_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
};