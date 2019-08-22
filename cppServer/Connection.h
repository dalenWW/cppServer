#pragma once
#include<functional>
#include<memory>
#include"base/noncopyable.h"
#include<string>
#include <netinet/in.h>


class EventLoop;
class Channel;

class Connection : noncopyable,
            public std::enable_shared_from_this<Connection>
{
public:
      typedef std::function<void(std::shared_ptr<Connection>)> Callback;
      typedef std::function<void(std::shared_ptr<Connection>,std::string&)> MessageCallback;

      Connection(EventLoop* loop, int connfd, struct sockaddr_in local_addr,struct sockaddr_in peer_addr);
      ~Connection();
      
      // 在loop上注册事件，连接建立时调用
      void Register();
      // void send(const void* data, size_t len);
      void send(const std::string& message);
      void sendInLoop();

      void shutdown();
      void shutdownInLoop();
      void handleRead();
      void handleWrite();
      void handleClose();
      void handleError();
            // 设置回调
      void setConnCb(Callback cb) { connCallback_ = std::move(cb); }
      void setMessageCb(MessageCallback cb) { messageCallback_ = std::move(cb); }
      void setWriteFinishCb(Callback cb) { writeFinishCallback_ = std::move(cb); }
      void setCloseCb(Callback cb) { closeCallback_ = std::move(cb); }
      void setConnRemove(Callback cb) { connRemove_ = std::move(cb); }
      int getFd() const { return connfd_; }

private:
      EventLoop* loop_;    // belongs to which loop
      const int connfd_;
      bool disconnected_;
      bool halfclose_;
      std::shared_ptr<Channel> connChannel_;

      struct sockaddr_in local_addr_;
      struct sockaddr_in peer_addr_;
      // 连接建立回调
      Callback connCallback_;
      // 消息到达
      MessageCallback messageCallback_;
      // 答复完成
      Callback writeFinishCallback_;
      // 连接关闭
      Callback closeCallback_;
      // 连接清理
      Callback connRemove_;

      std::string inBuffer_;
      std::string outBuffer_;

};
typedef std::shared_ptr<Connection> sp_Connection;
