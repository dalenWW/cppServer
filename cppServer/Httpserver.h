#pragma once
#include"base/MutexLock.h"
#include"EventLoop.h"
#include"EventLoopThreadPool.h"
#include"Server.h"
#include"Timer.h"
#include"Connection.h"
#include"Http.h"
#include<map>
#include"base/MutexLock.h"

class HttpServer
{
public:
      HttpServer(EventLoop *loop, const int port, int numThreads);
      ~HttpServer();

      void start();

private:
      //新连接回调函数
      void onConnection(const sp_Connection& conn); 
      //数据接收回调函数
      void onMessage(const sp_Connection&conn, std::string &msg); 
      //数据发送完成回调函数
      void onSend(const sp_Connection& sptcpconn); 
      //连接关闭回调函数
      void onClose(const sp_Connection& sptcpconn); 
      //管理Http会话
      std::map<sp_Connection, std::shared_ptr<Http> > httpConnlist_; 
      //管理定时器,维护活跃连接
      std::map<sp_Connection, sp_Timer> timerlist_; 
      MutexLock mutex_;
      Server server_; 
};


