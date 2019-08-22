#pragma once
#include"base/noncopyable.h"
#include<memory>
#include<functional>
#include<sys/epoll.h>

class EventLoop;

class Channel : noncopyable
{
public:
    typedef std::function<void()> CallBack;

    Channel(EventLoop *loop,int fd);
    Channel(EventLoop* loop);
    ~Channel();

    int getFd() const { return fd_; }
    void setFd(int fd) { fd_ = fd; }
    __uint32_t getEvents() const  { return events_; }
    void setEvents(__uint32_t events) { events_ = events; }
    void setRevents(__uint32_t revents) { revents_ = revents; }
    bool IsWriting() const { return events_ & EPOLLOUT; }

    void enableRead()    { events_ |= (EPOLLIN | EPOLLPRI); }
    void enableWrite()   { events_ |= EPOLLOUT; }
    void disableRead()   { events_ &= ~(EPOLLIN | EPOLLPRI); }
    void DisableWrite()  { events_ &= ~EPOLLOUT; }

    void setReadCallBack(CallBack readCallBack)
    {
        readCallBack_ = std::move(readCallBack);
    }

    void setWriteCallBack(CallBack writeCallBack)
    {
        writeCallBack_ = std::move(writeCallBack);
    }

    void setErrorCallBack(CallBack errorCallBack)
    {
        errorCallBack_ = std::move(errorCallBack);
    }

    void setCloseCallBack(CallBack closeCallBack)
    {
        closeCallBack_ = std::move(closeCallBack);
    }
    EventLoop* ownerLoop() { return loop_; }
 
    void handleEvent(); 

private:
    EventLoop* loop_;     //belongs to which eventloop
    int fd_;
    __uint32_t events_;
    __uint32_t revents_;

    CallBack readCallBack_;
    CallBack writeCallBack_;
    CallBack errorCallBack_;
    CallBack closeCallBack_;
             
};

typedef std::shared_ptr<Channel> sp_Channel;