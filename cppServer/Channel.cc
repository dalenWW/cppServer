#include"Channel.h"
#include<assert.h>
#include"EventLoop.h"
#include"base/Logging.h"

Channel::Channel(EventLoop* loop,int fd) 
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0)
{
}

Channel::Channel(EventLoop* loop)
    : loop_(loop),
      events_(0),
      revents_(0)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent()
{
    // 服务器端发生异常
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallBack_)
            closeCallBack_();
        return;
    }
    // 客户端关闭连接,有些会检测不到，所以不依赖它来关连接
    if (revents_ & EPOLLRDHUP)
    {
        LOG << "Event EPOLLRDHUP";
        if (closeCallBack_)
            closeCallBack_();
        return;
    }
    // 发生错误
    if (revents_ & EPOLLERR)
    {
        if (errorCallBack_)
            errorCallBack_();
        if (closeCallBack_)
            closeCallBack_();
        return;
    }    
    // 可读事件
    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallBack_)
            readCallBack_();
    }
    // 可写事件
    if (revents_ & EPOLLOUT)
    {
        if (writeCallBack_)
            writeCallBack_();
    }
}

