#pragma once
#include"base/noncopyable.h"
#include"base/CurrentThread.h"
#include"base/Thread.h"
#include<assert.h>
#include<memory>
#include<vector>
#include"Epoll.h"
#include"base/MutexLock.h"
#include"Util.h"

class Channel;
class TimerHeap;
class Timer;

class EventLoop : noncopyable
{
public:
    typedef std::function<void ()> Functor;
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void wakeup();
    void handleWakeup();
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void doPendingFunctors();
    void addChannel(Channel* channel) { poller_->epoll_add(channel); }
    void modChannel(Channel* channel) { poller_->epoll_mod(channel); }
    void removeChannel(Channel* channel) { poller_->epoll_del(channel); }
    void addTimer(std::shared_ptr<Timer>);

private:
    const pid_t threadId_;
    int wakeupFd_;
    bool quit_;
    bool callingPendingFunctors_;
    Channel* wakeupChannel_;   // sp_channel
    MutexLock mutex_;

    std::unique_ptr<Epoll> poller_;
    std::unique_ptr<TimerHeap> timerHeap_;
    std::vector<Functor> pendingFunctors_;
};
