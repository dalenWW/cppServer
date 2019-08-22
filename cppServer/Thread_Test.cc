#include"Thread.h"
#include"CurrentThread.h"
#include<string>
#include<functional>
#include<stdio.h>

void threadFunc1()
{
    printf("tid = %d\n",CurrentThread::tid());
}

void threadFunc2(int x)
{
    printf("tid = %d,x = %d\n",CurrentThread::tid(),x);
}

int main()
{
    Thread* t1 = new Thread(threadFunc1,"thread1");
    t1->start();
    t1->join();
    Thread* t2 = new Thread(std::bind(threadFunc2,5),"thread2");
    t2->start();
    t2->join();
}