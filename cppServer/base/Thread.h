#pragma once
#include"noncopyable.h"
#include<functional>
#include<pthread.h>
#include<string>

class Thread:noncopyable
{
public:
    typedef std::function<void ()> ThreadFunc;
    explicit Thread(const ThreadFunc&,const std::string& name=std::string());
    ~Thread();
    void start();
    int join();
    bool started() const {return started_;}
    const std::string& name() const {return name_;}
    pid_t tid() const {return tid_;}

private:
    void setDefaultName();
    bool started_;
    pid_t tid_;
    pthread_t pthreadId_;
    std::string name_;
    ThreadFunc func_;
};