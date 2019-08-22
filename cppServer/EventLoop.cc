#include"EventLoop.h"
#include<unistd.h>
#include"base/Logging.h"
#include<sys/eventfd.h>
#include"Channel.h"
#include"Timer.h"

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
    : threadId_(CurrentThread::tid()),
      quit_(false),
      callingPendingFunctors_(false),
      wakeupFd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      wakeupChannel_(new Channel(this,wakeupFd_)),
      poller_(new Epoll()),
      timerHeap_(new TimerHeap(this))
{
    if(wakeupFd_ < 0)
    {
        LOG << "create wakeupFd failed";
        abort();
    }
    if(t_loopInThisThread)
    {
        LOG << "Another EventLoop " << t_loopInThisThread << "exits in this thread " << threadId_; // not allowed 
    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleWakeup, this));
    wakeupChannel_->setEvents(EPOLLIN);
    addChannel(wakeupChannel_);
}

EventLoop::~EventLoop()
{
    t_loopInThisThread = NULL;
}
// 唤醒循环，由其他线程调用
void EventLoop::wakeup()
{
    uint64_t one = 1;     // 8 bytes
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}
// 处理唤醒
void EventLoop::handleWakeup()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

//  can be called in another thread
void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

// must be called in the thread created the object
void EventLoop::loop()
{
    assert(isInLoopThread());
    quit_ = false;
    std::vector<Channel*> activeChannels;
    while(!quit_)
    {
        activeChannels.clear();
        activeChannels = poller_->poll();
        for(auto it : activeChannels)
            it->handleEvent();
        doPendingFunctors();
        //    处理超时
        // timerHeap_->handleExpired();
    }
}

void EventLoop::doPendingFunctors()
{
    callingPendingFunctors_ = true;
    std::vector<Functor> functors;
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(size_t i = 0;i < functors.size();i++)
        functors[i]();
    callingPendingFunctors_ = false;
}

// 运行任务，可由其他线程调用
void EventLoop::runInLoop(Functor cb)
{
    // 如果不是在自己所在线程调用，则添加进任务队列后唤醒该loop进行处理
    if (isInLoopThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

// 添加任务
void EventLoop::queueInLoop(Functor cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if(!isInLoopThread() || callingPendingFunctors_)
        wakeup();
}

void EventLoop::addTimer(std::shared_ptr<Timer> timer)
{
    timerHeap_->addTimer(timer);
}
