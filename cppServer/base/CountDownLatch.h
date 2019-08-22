#pragma once
#include"Condition.h"
#include"noncopyable.h"
#include"MutexLock.h"

class CountDownLatch:noncopyable
{
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

private:
    MutexLock mutex_;
    Condition cond_;
    int count_;
};