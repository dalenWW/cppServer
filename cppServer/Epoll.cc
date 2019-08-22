#include"Epoll.h"
#include"Channel.h"
#include <unistd.h>
#include"base/Logging.h"

Epoll::Epoll()
    : epollfd_(epoll_create1(EPOLL_CLOEXEC)),
      activeEvent_(kEventNum)
{
    assert(epollfd_>0);
}

Epoll::~Epoll()
{
    // close(epollfd_);
}

void Epoll::epoll_add(Channel* channel)
{
    int fd = channel->getFd();
    struct epoll_event event;
    event.events = channel->getEvents();
    event.data.fd = fd;
    // 添加进映射表
    fd2Channel_[fd] = channel;
    // 往内核epoll事件表注册
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        LOG << "epoll_ctl add failed";
        // 注册失败，从映射表中清除
        fd2Channel_.erase(fd);
        exit(1);
    }
    //   test
    LOG << "epoll add fd = " << fd;
}

void Epoll::epoll_mod(Channel* channel)
{
    int fd = channel->getFd();
    struct epoll_event event;
    event.events = channel->getEvents();
    event.data.fd = fd;

    if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        LOG << "epoll_ctl mod failed";
        fd2Channel_.erase(fd);
    }
}

void Epoll::epoll_del(Channel* channel)
{
    int fd = channel->getFd();
    struct epoll_event event;
    event.events = channel->getEvents();
    event.data.fd = fd;

    if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        LOG << "epoll_ctl del failed";
    }
    fd2Channel_.erase(fd);
}

std::vector<Channel*> Epoll::poll()
{
    // 发生的活跃事件将会把epoll_event结构体放到activeEvent_中去
    int active_event_count = epoll_wait(epollfd_, &*activeEvent_.begin(), activeEvent_.size(), kEpollWaitTime);

    if (active_event_count < 0)
        LOG << "epoll_wait error";
    
    std::vector<Channel*> activeChannelList;
    for (int i = 0; i < active_event_count; ++i)
    {
        // 从映射表中取出eventbase
        Channel* eventbase = fd2Channel_[activeEvent_[i].data.fd];
        // 设置eventbase的活跃事件
        eventbase->setRevents(activeEvent_[i].events);

        activeChannelList.push_back(eventbase);
    }

    return activeChannelList;
}
