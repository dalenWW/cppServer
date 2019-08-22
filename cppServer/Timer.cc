#include"Timer.h"
#include"base/Logging.h"
#include <sys/timerfd.h>
#include"Channel.h"
#include <unistd.h>
#include"EventLoop.h"

TimerHeap::TimerHeap(EventLoop* loop)
    : loop_(loop),
      timerfd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      timerfdChannel_(std::make_shared<Channel>(loop_,timerfd_))
{
    timerfdChannel_->setReadCallBack(std::bind(&TimerHeap::handleExpired,this));
    timerfdChannel_->setEvents(EPOLLIN | EPOLLPRI);
    loop->addChannel(timerfdChannel_.get());
}

TimerHeap::~TimerHeap()
{
    close(timerfd_);
}

void TimerHeap::addTimer(sp_Timer timer)
{
    // 新的定时器到期时间小于队列中最小的那个的话，则更新timerfd到期时间
    if (timerQueue_.empty() || timer->getExpiredTime() < timerQueue_.top()->getExpiredTime())
    {
        updateTimerfd(timerfd_, timer->getExpiredTime());
    }
    timerQueue_.push(timer);
}

void TimerHeap::updateTimerfd(int timerfd,Timestamp when)
{
    // 计算多久后到期
    struct timespec ts;
    Nanosecond gap = when - system_clock::now();   //  ns
    ts.tv_sec = static_cast<time_t>(gap.count() / std::nano::den);
    ts.tv_nsec = gap.count() % std::nano::den;

    // 更新timerfd剩余到期时间
    struct itimerspec new_value, old_value;
    bzero(&new_value, sizeof(new_value));
    bzero(&old_value, sizeof(old_value));

    new_value.it_value = ts;
    timerfd_settime(timerfd, 0, &new_value, &old_value);
}

void TimerHeap::handleExpired()
{
    // read the timerfd
    uint64_t exp_cnt;
    ssize_t n = read(timerfd_, &exp_cnt, sizeof(exp_cnt));
    if (n != sizeof(exp_cnt))
    {
        LOG << "read error and errno is " << errno;
    }

    Timestamp now(system_clock::now());
    // 处理所有到期的定时器
    while (!timerQueue_.empty() && timerQueue_.top()->getExpiredTime() < now)
    {
        timerQueue_.top()->run();
        timerQueue_.pop();
    }
    // 重设timerfd到期时间
    if (!timerQueue_.empty())
    {
        updateTimerfd(timerfd_, timerQueue_.top()->getExpiredTime());
    }
}
