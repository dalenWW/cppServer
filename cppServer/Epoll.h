#pragma once
#include<sys/epoll.h>
#include<vector>
#include<memory>
#include<map>

class Channel;

class Epoll
{
public:
    Epoll();
    ~Epoll();
        // 往epoll的事件表上注册事件
    void epoll_add(Channel* channel);
    void epoll_mod(Channel* channel);
    void epoll_del(Channel* channel);
    std::vector<Channel* > poll();

private:
    int epollfd_;
    std::vector<epoll_event> activeEvent_;
      // 从fd到channel的映射列表
    std::map<int, Channel* > fd2Channel_;
    static const int kEventNum = 4096;
    const int kEpollWaitTime = 10000;    //   ms 
};