#include"Httpserver.h"

HttpServer::HttpServer(EventLoop *loop, const int port, int numThreads)
      : server_(loop,port,numThreads)
{
      server_.setConnCb(std::bind(&HttpServer::onConnection,this,std::placeholders::_1));
      server_.setMessageCb(std::bind(&HttpServer::onMessage,this,std::placeholders::_1,std::placeholders::_2));
      server_.setWriteFinishCb(std::bind(&HttpServer::onSend,this,std::placeholders::_1));
      server_.setCloseCb(std::bind(&HttpServer::onClose,this,std::placeholders::_1));
}

HttpServer::~HttpServer()
{
}

void HttpServer::onConnection(const sp_Connection& conn)
{
      auto spHttp = std::make_shared<Http>();
      sp_Timer timer = std::make_shared<Timer>(std::bind(&Connection::shutdown,conn),Nanosecond(5000000000));   // 5s
      server_.getLoop()->addTimer(timer);
      {
            MutexLockGuard lock(mutex_);
            httpConnlist_[conn] = spHttp;
            timerlist_[conn] = timer;
      }
}

void HttpServer::onMessage(const sp_Connection& conn, std::string &msg)
{
      std::shared_ptr<Http> spHttp;
      sp_Timer timer;
      {
            MutexLockGuard lock(mutex_);
            spHttp = httpConnlist_[conn];
            timer = timerlist_[conn];
      }
      timer->setExpiredTime(Nanosecond(5000000000));    //  重新设置成5s
      
      HttpRequest httpRequest;      
      std::string responsecontext;
      bool ret = spHttp->parseHttpRequest(msg);
      if(!ret)
      {
            spHttp->httpError(400,"Bad Request",responsecontext);
            conn->send(responsecontext);
            return;
      }
      //  解析成功
      std::string responsemsg;
      spHttp->httpHandle(responsemsg);
      conn->send(responsemsg);
      // 短连接
      if(!spHttp->isKeepAlive())
      {
      }
}

void HttpServer::start()
{
      LOG << "server start!";
      server_.start();
}

void HttpServer::onClose(const sp_Connection& conn)
{
      {
            MutexLockGuard lock(mutex_);
            httpConnlist_.erase(conn);
            timerlist_.erase(conn);
      }
}

void HttpServer::onSend(const sp_Connection& sptcpconn)
{
      
}