#pragma once
#include"base/noncopyable.h"
#include<queue>
#include<functional>
#include"base/Timestamp.h"
#include<memory>

class Channel;
class EventLoop;

class Timer : noncopyable
{
public:
    Timer(std::function<void()> cb, Nanosecond delay)
        : callback_(std::move(cb))
          {
              Timestamp now = system_clock::now();
              expiredTime_ = now + delay;
          }

    ~Timer() { }
    void run() const { callback_(); }
    Timestamp getExpiredTime() const { return expiredTime_; }
    void setExpiredTime(Nanosecond delay) { expiredTime_ = system_clock::now() + delay; }
    
private:
    std::function<void()> callback_;
    Timestamp expiredTime_;     // absolute time (ns)
};
typedef std::shared_ptr<Timer> sp_Timer;

struct TimerCmp
{
    bool operator()(const sp_Timer &a,const sp_Timer &b) const
    {
        return a->getExpiredTime() > b->getExpiredTime();
    }
};

class TimerHeap : noncopyable
{
public:
    TimerHeap(EventLoop* loop);
    ~TimerHeap();
    void addTimer(sp_Timer timer);
    void handleExpired();
    void updateTimerfd(int timerfd,Timestamp when);

private:
    EventLoop* loop_;
    int timerfd_;
    std::shared_ptr<Channel> timerfdChannel_;
    std::priority_queue<sp_Timer,std::vector<sp_Timer>,TimerCmp> timerQueue_;
};