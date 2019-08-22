#include"EventLoopThreadPool.h"
#include"EventLoopThread.h"
#include"EventLoop.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,int numThreads)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(numThreads),
      next_(0)
{
    if (numThreads_ <= 0)
    {
        LOG << "numThreads_ <= 0";
        abort();
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
    assert(baseLoop_->isInLoopThread());
    started_ = true;
    for(int i = 0;i < numThreads_;i++) {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    assert(baseLoop_->isInLoopThread());
    assert(started_);
    EventLoop* loop = baseLoop_;
    // round-robin
    if(!loops_.empty())
    {
        loop = loops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}