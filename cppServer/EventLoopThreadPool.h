#pragma once
#include"base/noncopyable.h"
#include"base/Logging.h"
#include<memory>
#include<vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop,int numThreads);
    ~EventLoopThreadPool();
    void start();
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread> > threads_;
    std::vector<EventLoop*> loops_;
};
